// ─────────────────────────────────────────────────────────────────
// diagnostic_screen.dart
//
// Shows live diagnostic progress as NJQCA.exe runs tests.
// Launched after inspection type is selected.
//
// Flow:
//   initState → reads route args (or restartStateProvider on restart)
//             → calls notifier.start() which writes files + launches NJQCA.exe
//   Stream    → each NjEvent updates state → test cards rebuild
//   interactive_needed → overlay shown, user confirms → ack file written
//   session_complete   → navigates to /results
// ─────────────────────────────────────────────────────────────────

import 'dart:async';
import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../core/nj_theme.dart';
import '../../core/njqca_runner.dart';
import '../../core/event_model.dart';
import '../auth/auth_service.dart' show kAppVersion;

// ═══════════════════════════════════════════════════════════════════
// STATE
// ═══════════════════════════════════════════════════════════════════

enum TestStatus { waiting, running, done, pass, fail, skipped }

class TestEntry {
  final String testId;
  final String label;
  final String part;
  final TestStatus status;
  final int? score;
  final String? progressMsg;

  const TestEntry({
    required this.testId,
    required this.label,
    required this.part,
    this.status = TestStatus.waiting,
    this.score,
    this.progressMsg,
  });

  TestEntry copyWith({
    TestStatus? status,
    int? score,
    String? progressMsg,
    bool clearProgress = false,
  }) =>
      TestEntry(
        testId:      testId,
        label:       label,
        part:        part,
        status:      status      ?? this.status,
        score:       score       ?? this.score,
        progressMsg: clearProgress ? null : (progressMsg ?? this.progressMsg),
      );
}

enum _Phase { launching, running, interactive, complete, error }

class DiagnosticState {
  final _Phase phase;
  final List<TestEntry> tests;
  final String? serial;
  final String? inspection;
  final String? user;
  final String? department;
  final NjInteractiveNeeded? interactiveEvent;
  final String? erpUrl;
  final int finalScore;
  final String? errorMessage;

  const DiagnosticState({
    this.phase = _Phase.launching,
    this.tests = const [],
    this.serial,
    this.inspection,
    this.user,
    this.department,
    this.interactiveEvent,
    this.erpUrl,
    this.finalScore = 0,
    this.errorMessage,
  });

  int get completedCount => tests.where((t) =>
      t.status == TestStatus.done ||
      t.status == TestStatus.pass ||
      t.status == TestStatus.fail ||
      t.status == TestStatus.skipped).length;

  int get totalCount => tests.length;

  DiagnosticState copyWith({
    _Phase? phase,
    List<TestEntry>? tests,
    String? serial,
    String? inspection,
    String? user,
    String? department,
    NjInteractiveNeeded? interactiveEvent,
    bool clearInteractive = false,
    String? erpUrl,
    int? finalScore,
    String? errorMessage,
  }) =>
      DiagnosticState(
        phase:          phase          ?? this.phase,
        tests:          tests          ?? this.tests,
        serial:         serial         ?? this.serial,
        inspection:     inspection     ?? this.inspection,
        user:           user           ?? this.user,
        department:     department     ?? this.department,
        interactiveEvent: clearInteractive
            ? null
            : (interactiveEvent ?? this.interactiveEvent),
        erpUrl:         erpUrl         ?? this.erpUrl,
        finalScore:     finalScore     ?? this.finalScore,
        errorMessage:   errorMessage   ?? this.errorMessage,
      );
}

// ═══════════════════════════════════════════════════════════════════
// NOTIFIER
// ═══════════════════════════════════════════════════════════════════

class DiagnosticNotifier extends StateNotifier<DiagnosticState> {
  DiagnosticNotifier() : super(const DiagnosticState());

  final _runner = NjqcaRunner();
  StreamSubscription<NjEvent>? _sub;
  bool _started = false;

  Future<void> start({
    required String username,
    required String password,
    required String inspectionType,
  }) async {
    if (_started) return;
    _started = true;

    state = state.copyWith(
      phase:      _Phase.launching,
      user:       username,
      inspection: inspectionType,
    );

    final stream = _runner.launch(
      username:       username,
      password:       password,
      inspectionType: inspectionType,
    );

    _sub = stream.listen(
      _onEvent,
      onError: (e) => state = state.copyWith(
        phase:        _Phase.error,
        errorMessage: 'Stream error: $e',
      ),
      onDone: () {
        if (state.phase != _Phase.complete) {
          state = state.copyWith(
            phase:        _Phase.error,
            errorMessage: 'NJQCA.exe exited unexpectedly.',
          );
        }
      },
    );
  }

  // ── Event handler ───────────────────────────────────────────────
  void _onEvent(NjEvent event) {
    if (event is NjSessionStart) {
      state = state.copyWith(
        phase:      _Phase.running,
        serial:     event.serial,
        user:       event.user,
        inspection: event.inspection,
        department: event.department,
      );
      return;
    }

    if (event is NjTestStart) {
      final updated = List<TestEntry>.from(state.tests);
      final idx = updated.indexWhere((t) => t.testId == event.test);
      if (idx >= 0) {
        updated[idx] = updated[idx].copyWith(status: TestStatus.running);
      } else {
        updated.add(TestEntry(
          testId: event.test,
          label:  event.label.isNotEmpty ? event.label : event.test,
          part:   event.part,
          status: TestStatus.running,
        ));
      }
      state = state.copyWith(phase: _Phase.running, tests: updated);
      return;
    }

    if (event is NjTestProgress) {
      final updated = List<TestEntry>.from(state.tests);
      final idx = updated.indexWhere((t) => t.testId == event.test);
      if (idx >= 0) {
        updated[idx] = updated[idx].copyWith(progressMsg: event.message);
        state = state.copyWith(tests: updated);
      }
      return;
    }

    if (event is NjTestDone) {
      final updated = List<TestEntry>.from(state.tests);
      final idx = updated.indexWhere((t) => t.testId == event.test);
      final status = event.isPassed
          ? TestStatus.pass
          : event.isFailed
              ? TestStatus.fail
              : TestStatus.done;
      if (idx >= 0) {
        updated[idx] = updated[idx].copyWith(
          status:        status,
          score:         event.score > 0 ? event.score : null,
          clearProgress: true,
        );
      } else {
        updated.add(TestEntry(
          testId: event.test,
          label:  event.test,
          part:   '',
          status: status,
          score:  event.score > 0 ? event.score : null,
        ));
      }
      state = state.copyWith(tests: updated);
      return;
    }

    if (event is NjTestSkipped) {
      final updated = List<TestEntry>.from(state.tests);
      final idx = updated.indexWhere((t) => t.testId == event.test);
      if (idx >= 0) {
        updated[idx] = updated[idx].copyWith(status: TestStatus.skipped);
        state = state.copyWith(tests: updated);
      }
      return;
    }

    if (event is NjInteractiveNeeded) {
      state = state.copyWith(
        phase:          _Phase.interactive,
        interactiveEvent: event,
      );
      return;
    }

    if (event is NjPerformanceStart) {
      final updated = List<TestEntry>.from(state.tests);
      final idx = updated.indexWhere((t) => t.testId == event.test);
      if (idx >= 0) {
        updated[idx] = updated[idx].copyWith(status: TestStatus.running);
        state = state.copyWith(tests: updated);
      }
      return;
    }

    if (event is NjRestartScheduled) {
      // Write restart state so Flutter resumes after reboot
      // NJQCA handles the actual restart — we just update the UI briefly
      state = state.copyWith(
        errorMessage: 'Restarting (${event.restartNumber}/${event.maxRestarts})...',
      );
      return;
    }

    if (event is NjSessionComplete) {
      state = state.copyWith(
        phase:      _Phase.complete,
        erpUrl:     event.erpUrl,
        finalScore: event.score,
      );
      return;
    }

    if (event is NjErrorEvent) {
      // Non-fatal errors (battery, score mapping) show as a banner but don't stop the stream
      state = state.copyWith(
        phase:        _Phase.error,
        errorMessage: event.message.isNotEmpty ? event.message : 'An error occurred.',
      );
      return;
    }
  }

  // ── Interactive acknowledgement ─────────────────────────────────
  Future<void> acknowledge(String testId, {required bool passed}) async {
    await _runner.acknowledgeInteractive(testId, passed: passed);
    state = state.copyWith(
      phase:            _Phase.running,
      clearInteractive: true,
    );
  }

  @override
  void dispose() {
    _sub?.cancel();
    _runner.kill();
    super.dispose();
  }
}

final diagnosticProvider =
    StateNotifierProvider.autoDispose<DiagnosticNotifier, DiagnosticState>(
  (ref) => DiagnosticNotifier(),
);

// Provider for restart state — defined here, overridden in main.dart on restart
final restartStateProvider = Provider<RestartState?>((ref) => null);

// ═══════════════════════════════════════════════════════════════════
// SCREEN
// ═══════════════════════════════════════════════════════════════════

class DiagnosticScreen extends ConsumerStatefulWidget {
  const DiagnosticScreen({super.key});

  @override
  ConsumerState<DiagnosticScreen> createState() => _DiagnosticScreenState();
}

class _DiagnosticScreenState extends ConsumerState<DiagnosticScreen> {
  final _scrollCtrl = ScrollController();

  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addPostFrameCallback((_) => _startSession());
  }

  @override
  void dispose() {
    _scrollCtrl.dispose();
    super.dispose();
  }

  void _startSession() {
    // Restart resume — check restartStateProvider first
    final restart = ref.read(restartStateProvider);
    if (restart != null) {
      ref.read(diagnosticProvider.notifier).start(
        username:       restart.username,
        password:       restart.password,
        inspectionType: restart.inspectionType,
      );
      return;
    }

    // Normal flow — read from route arguments
    final args =
        ModalRoute.of(context)?.settings.arguments as Map<String, dynamic>?;
    final username       = args?['username']       as String? ?? '';
    final password       = args?['password']       as String? ?? '';
    final inspectionType = args?['inspectionType'] as String? ?? '';

    ref.read(diagnosticProvider.notifier).start(
      username:       username,
      password:       password,
      inspectionType: inspectionType,
    );
  }

  @override
  Widget build(BuildContext context) {
    final st = ref.watch(diagnosticProvider);

    // Navigate to results when session completes
    ref.listen<DiagnosticState>(diagnosticProvider, (prev, next) {
      if (prev?.phase != _Phase.complete && next.phase == _Phase.complete) {
        Navigator.of(context).pushReplacementNamed(
          '/results',
          arguments: {
            'erpUrl':     next.erpUrl ?? '',
            'score':      next.finalScore,
            'inspection': next.inspection ?? '',
            'serial':     next.serial ?? '',
            'tests':      next.tests,
          },
        );
      }

      // Auto-scroll to bottom when new test added
      if ((prev?.tests.length ?? 0) < next.tests.length) {
        WidgetsBinding.instance.addPostFrameCallback((_) {
          if (_scrollCtrl.hasClients) {
            _scrollCtrl.animateTo(
              _scrollCtrl.position.maxScrollExtent,
              duration: const Duration(milliseconds: 300),
              curve: Curves.easeOut,
            );
          }
        });
      }
    });

    return Scaffold(
      body: Stack(
        children: [
          // ── Main content
          Row(
            children: [
              SizedBox(
                width: 300,
                child: _LeftPanel(state: st),
              ),
              Expanded(
                child: _RightPanel(state: st, scrollCtrl: _scrollCtrl),
              ),
            ],
          ),

          // ── Interactive overlay
          if (st.phase == _Phase.interactive && st.interactiveEvent != null)
            _InteractiveOverlay(
              event: st.interactiveEvent!,
              onAcknowledge: (passed) => ref
                  .read(diagnosticProvider.notifier)
                  .acknowledge(st.interactiveEvent!.test, passed: passed),
            ),
        ],
      ),
    );
  }
}

// ═══════════════════════════════════════════════════════════════════
// LEFT PANEL
// ═══════════════════════════════════════════════════════════════════

class _LeftPanel extends StatelessWidget {
  final DiagnosticState state;
  const _LeftPanel({required this.state});

  @override
  Widget build(BuildContext context) {
    final completed = state.completedCount;
    final total     = state.totalCount;
    final progress  = total > 0 ? completed / total : null;

    return Stack(
      children: [
        Positioned.fill(child: ColoredBox(color: NJColors.primary)),
        Positioned.fill(
          child: DecoratedBox(
            decoration: BoxDecoration(
              gradient: RadialGradient(
                center: const Alignment(1.0, -1.0),
                radius: 1.2,
                colors: [
                  Colors.white.withOpacity(0.10),
                  Colors.transparent,
                ],
              ),
            ),
          ),
        ),

        Padding(
          padding: const EdgeInsets.all(NJSpacing.xl),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              const SizedBox(height: NJSpacing.sm),

              // Logo
              _NJLogo(),

              const SizedBox(height: NJSpacing.xxl),

              // Inspection type label
              Text(
                'Inspection type',
                style: NJText.bodySm(
                    color: NJColors.onPrimary.withOpacity(0.55)),
              ),
              const SizedBox(height: NJSpacing.xs),
              Text(
                state.inspection ?? '—',
                style: NJText.titleLg(color: NJColors.onPrimary),
                maxLines: 2,
                overflow: TextOverflow.ellipsis,
              ),

              const SizedBox(height: NJSpacing.lg),

              // Serial number (appears after session_start)
              if (state.serial != null && state.serial!.isNotEmpty) ...[
                _InfoRow(
                  icon: Icons.laptop_outlined,
                  label: state.serial!,
                ),
                const SizedBox(height: NJSpacing.sm),
              ],

              // Department badge
              if (state.department != null &&
                  state.department!.isNotEmpty)
                Container(
                  padding: const EdgeInsets.symmetric(
                    horizontal: NJSpacing.md,
                    vertical: NJSpacing.xs,
                  ),
                  decoration: BoxDecoration(
                    color: Colors.white.withOpacity(0.15),
                    borderRadius: BorderRadius.circular(NJRadius.pill),
                  ),
                  child: Row(
                    mainAxisSize: MainAxisSize.min,
                    children: [
                      Icon(Icons.badge_outlined,
                          size: 13,
                          color: NJColors.onPrimary.withOpacity(0.7)),
                      const SizedBox(width: NJSpacing.xs),
                      Text(
                        state.department!,
                        style: NJText.labelSm(
                            color: NJColors.onPrimary.withOpacity(0.9)),
                      ),
                    ],
                  ),
                ),

              const Spacer(),

              // Progress section
              if (total > 0) ...[
                Text(
                  '$completed / $total tests',
                  style: NJText.labelLg(color: NJColors.onPrimary),
                ),
                const SizedBox(height: NJSpacing.sm),
                ClipRRect(
                  borderRadius: BorderRadius.circular(NJRadius.pill),
                  child: LinearProgressIndicator(
                    value: progress,
                    minHeight: 6,
                    backgroundColor: Colors.white.withOpacity(0.2),
                    valueColor:
                        const AlwaysStoppedAnimation(NJColors.onPrimary),
                  ),
                ),
                const SizedBox(height: NJSpacing.sm),
                Text(
                  state.phase == _Phase.complete
                      ? 'Session complete'
                      : state.phase == _Phase.interactive
                          ? 'Waiting for input...'
                          : 'Running...',
                  style: NJText.bodySm(
                      color: NJColors.onPrimary.withOpacity(0.6)),
                ),
              ] else ...[
                Row(
                  children: [
                    SizedBox(
                      width: 14,
                      height: 14,
                      child: CircularProgressIndicator(
                        strokeWidth: 1.5,
                        color: NJColors.onPrimary.withOpacity(0.6),
                      ),
                    ),
                    const SizedBox(width: NJSpacing.sm),
                    Text(
                      'Launching NJQCA...',
                      style: NJText.bodySm(
                          color: NJColors.onPrimary.withOpacity(0.6)),
                    ),
                  ],
                ),
              ],

              const SizedBox(height: NJSpacing.xl),
            ],
          ),
        ),

        // Version
        Positioned(
          left: NJSpacing.xl,
          bottom: NJSpacing.lg,
          child: Text(
            'v${kAppVersion.toStringAsFixed(1)}',
            style: NJText.labelSm(
                color: NJColors.onPrimary.withOpacity(0.35)),
          ),
        ),
      ],
    );
  }
}

class _NJLogo extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(
          horizontal: NJSpacing.md, vertical: NJSpacing.sm),
      decoration: BoxDecoration(
        color: Colors.white.withOpacity(0.14),
        borderRadius: BorderRadius.circular(NJRadius.md),
      ),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          Container(
            width: 34, height: 34,
            decoration: BoxDecoration(
              color: NJColors.onPrimary,
              borderRadius: BorderRadius.circular(NJRadius.xs),
            ),
            child: Center(
              child: Text('NJ', style: NJText.labelLg(color: NJColors.primary)),
            ),
          ),
          const SizedBox(width: NJSpacing.sm),
          Text('NewJaisa',
              style: NJText.titleLg(color: NJColors.onPrimary)
                  .copyWith(fontWeight: FontWeight.w700)),
        ],
      ),
    );
  }
}

class _InfoRow extends StatelessWidget {
  final IconData icon;
  final String label;
  const _InfoRow({required this.icon, required this.label});

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        Icon(icon, size: 14, color: NJColors.onPrimary.withOpacity(0.6)),
        const SizedBox(width: NJSpacing.xs),
        Expanded(
          child: Text(
            label,
            style: NJText.bodySm(color: NJColors.onPrimary.withOpacity(0.85)),
            overflow: TextOverflow.ellipsis,
          ),
        ),
      ],
    );
  }
}

// ═══════════════════════════════════════════════════════════════════
// RIGHT PANEL
// ═══════════════════════════════════════════════════════════════════

class _RightPanel extends StatelessWidget {
  final DiagnosticState state;
  final ScrollController scrollCtrl;

  const _RightPanel({required this.state, required this.scrollCtrl});

  @override
  Widget build(BuildContext context) {
    return Container(
      color: NJColors.surfaceContainerLowest,
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          // ── Header ─────────────────────────────────────────────
          Padding(
            padding: const EdgeInsets.fromLTRB(
                NJSpacing.xxl, NJSpacing.xxl, NJSpacing.xxl, NJSpacing.lg),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(_headerTitle(state.phase), style: NJText.headlineLg()),
                const SizedBox(height: NJSpacing.xs),
                Text(_headerSub(state), style: NJText.bodyMd()),

                // Error banner
                if (state.errorMessage != null &&
                    state.phase == _Phase.error) ...[
                  const SizedBox(height: NJSpacing.md),
                  _ErrorBanner(message: state.errorMessage!),
                ],
              ],
            ),
          ),

          // ── Test list ───────────────────────────────────────────
          Expanded(
            child: state.tests.isEmpty
                ? _LaunchingPlaceholder(phase: state.phase)
                : ListView.builder(
                    controller: scrollCtrl,
                    padding: const EdgeInsets.fromLTRB(
                        NJSpacing.xxl, 0, NJSpacing.xxl, NJSpacing.xxl),
                    itemCount: state.tests.length,
                    itemBuilder: (_, i) => Padding(
                      padding:
                          const EdgeInsets.only(bottom: NJSpacing.sm),
                      child: _TestCard(entry: state.tests[i]),
                    ),
                  ),
          ),
        ],
      ),
    );
  }

  String _headerTitle(_Phase phase) {
    switch (phase) {
      case _Phase.launching:   return 'Preparing session';
      case _Phase.running:     return 'Running diagnostics';
      case _Phase.interactive: return 'Input required';
      case _Phase.complete:    return 'Session complete';
      case _Phase.error:       return 'Session error';
    }
  }

  String _headerSub(DiagnosticState s) {
    if (s.phase == _Phase.launching) return 'Launching NJQCA.exe...';
    if (s.phase == _Phase.complete) {
      return 'All tests finished. View results in ERP.';
    }
    if (s.phase == _Phase.interactive) {
      return 'Complete the interactive test to continue.';
    }
    if (s.tests.isEmpty) return 'Waiting for first test...';
    final running = s.tests.where((t) => t.status == TestStatus.running).length;
    if (running > 0) return '$running test${running > 1 ? "s" : ""} running...';
    return '${s.completedCount} of ${s.totalCount} complete.';
  }
}

// ── Launching placeholder ─────────────────────────────────────────
class _LaunchingPlaceholder extends StatelessWidget {
  final _Phase phase;
  const _LaunchingPlaceholder({required this.phase});

  @override
  Widget build(BuildContext context) {
    return Center(
      child: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          if (phase == _Phase.error)
            const Icon(Icons.error_outline_rounded,
                size: 48, color: NJColors.error)
          else
            const SizedBox(
              width: 40, height: 40,
              child: CircularProgressIndicator(
                  strokeWidth: 3, color: NJColors.primary),
            ),
          const SizedBox(height: NJSpacing.lg),
          Text(
            phase == _Phase.error
                ? 'See error above'
                : 'Waiting for NJQCA to start tests...',
            style: NJText.bodyMd(),
          ),
        ],
      ),
    );
  }
}

// ─── Error banner ─────────────────────────────────────────────────
class _ErrorBanner extends StatelessWidget {
  final String message;
  const _ErrorBanner({required this.message});

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(
          horizontal: NJSpacing.md, vertical: NJSpacing.sm),
      decoration: BoxDecoration(
        color: NJColors.errorContainer,
        borderRadius: BorderRadius.circular(NJRadius.sm),
        border: Border.all(color: NJColors.error.withOpacity(0.3)),
      ),
      child: Row(
        children: [
          const Icon(Icons.error_outline_rounded,
              size: 16, color: NJColors.onErrorContainer),
          const SizedBox(width: NJSpacing.sm),
          Expanded(
            child: Text(message,
                style: NJText.bodySm(color: NJColors.onErrorContainer)),
          ),
        ],
      ),
    );
  }
}

// ═══════════════════════════════════════════════════════════════════
// TEST CARD
// ═══════════════════════════════════════════════════════════════════

class _TestCard extends StatelessWidget {
  final TestEntry entry;
  const _TestCard({required this.entry});

  @override
  Widget build(BuildContext context) {
    final isRunning = entry.status == TestStatus.running;

    return AnimatedContainer(
      duration: const Duration(milliseconds: 200),
      height: entry.progressMsg != null ? 82 : 68,
      decoration: BoxDecoration(
        color: isRunning
            ? NJColors.primaryFixed.withOpacity(0.25)
            : NJColors.surfaceContainerLowest,
        borderRadius: BorderRadius.circular(NJRadius.lg),
        border: Border.all(
          color: isRunning ? NJColors.primary : NJColors.outlineVariant,
          width: isRunning ? 1.5 : 1.0,
        ),
      ),
      child: Padding(
        padding: const EdgeInsets.symmetric(horizontal: NJSpacing.lg),
        child: Row(
          children: [
            // Part icon circle
            _PartIcon(part: entry.part),

            const SizedBox(width: NJSpacing.md),

            // Label + part + progress msg
            Expanded(
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    entry.label,
                    style: NJText.titleMd(),
                    maxLines: 1,
                    overflow: TextOverflow.ellipsis,
                  ),
                  if (entry.part.isNotEmpty)
                    Text(entry.part,
                        style: NJText.bodySm(),
                        maxLines: 1,
                        overflow: TextOverflow.ellipsis),
                  if (entry.progressMsg != null)
                    Text(
                      entry.progressMsg!,
                      style: NJText.bodySm()
                          .copyWith(fontStyle: FontStyle.italic),
                      maxLines: 1,
                      overflow: TextOverflow.ellipsis,
                    ),
                ],
              ),
            ),

            const SizedBox(width: NJSpacing.md),

            // Score badge
            if (entry.score != null && entry.score! > 0)
              Padding(
                padding: const EdgeInsets.only(right: NJSpacing.sm),
                child: Text(
                  '${entry.score}',
                  style: NJText.labelSm(color: NJColors.onSurfaceVariant),
                ),
              ),

            // Status indicator
            _StatusWidget(status: entry.status),
          ],
        ),
      ),
    );
  }
}

// ── Part icon circle ──────────────────────────────────────────────
class _PartIcon extends StatelessWidget {
  final String part;
  const _PartIcon({required this.part});

  @override
  Widget build(BuildContext context) {
    return Container(
      width: 44, height: 44,
      decoration: BoxDecoration(
        color: _bgColor,
        shape: BoxShape.circle,
      ),
      child: Icon(_icon, size: 20, color: _iconColor),
    );
  }

  IconData get _icon {
    switch (part.toLowerCase()) {
      case 'storage':     return Icons.storage_outlined;
      case 'ram':         return Icons.memory_outlined;
      case 'cpu':         return Icons.speed_outlined;
      case 'battery':     return Icons.battery_charging_full_outlined;
      case 'wifi':        return Icons.wifi_outlined;
      case 'bluetooth':   return Icons.bluetooth_outlined;
      case 'motherboard': return Icons.developer_board_outlined;
      default:            return Icons.settings_outlined;
    }
  }

  Color get _bgColor {
    switch (part.toLowerCase()) {
      case 'storage':     return const Color(0xFFFFF3E0);
      case 'ram':         return const Color(0xFFEDE7F6);
      case 'cpu':         return const Color(0xFFE0F7FA);
      case 'battery':     return const Color(0xFFE8F5E9);
      case 'wifi':        return const Color(0xFFE3F2FD);
      case 'bluetooth':   return const Color(0xFFE8EAF6);
      case 'motherboard': return const Color(0xFFFFF8E1);
      default:            return NJColors.primaryFixed;
    }
  }

  Color get _iconColor {
    switch (part.toLowerCase()) {
      case 'storage':     return const Color(0xFFE65100);
      case 'ram':         return const Color(0xFF6A1B9A);
      case 'cpu':         return const Color(0xFF00838F);
      case 'battery':     return const Color(0xFF2E7D32);
      case 'wifi':        return const Color(0xFF1565C0);
      case 'bluetooth':   return const Color(0xFF283593);
      case 'motherboard': return const Color(0xFFF57C00);
      default:            return NJColors.primary;
    }
  }
}

// ── Status indicator ──────────────────────────────────────────────
class _StatusWidget extends StatelessWidget {
  final TestStatus status;
  const _StatusWidget({required this.status});

  @override
  Widget build(BuildContext context) {
    switch (status) {
      case TestStatus.waiting:
        return const SizedBox(width: 22, height: 22);

      case TestStatus.running:
        return const SizedBox(
          width: 20, height: 20,
          child: CircularProgressIndicator(
            strokeWidth: 2.0,
            color: NJColors.primary,
          ),
        );

      case TestStatus.pass:
        return const Icon(Icons.check_circle_rounded,
            size: 22, color: Color(0xFF2E7D32));

      case TestStatus.fail:
        return const Icon(Icons.cancel_rounded,
            size: 22, color: NJColors.error);

      case TestStatus.done:
        return Icon(Icons.check_circle_outline_rounded,
            size: 22, color: NJColors.outline);

      case TestStatus.skipped:
        return Icon(Icons.remove_circle_outline_rounded,
            size: 22, color: NJColors.outline);
    }
  }
}

// ═══════════════════════════════════════════════════════════════════
// INTERACTIVE OVERLAY
// ═══════════════════════════════════════════════════════════════════

class _InteractiveOverlay extends StatelessWidget {
  final NjInteractiveNeeded event;
  final void Function(bool passed) onAcknowledge;

  const _InteractiveOverlay({
    required this.event,
    required this.onAcknowledge,
  });

  @override
  Widget build(BuildContext context) {
    return Positioned.fill(
      child: AnimatedOpacity(
        opacity: 1.0,
        duration: const Duration(milliseconds: 200),
        child: ColoredBox(
          color: Colors.black.withOpacity(0.55),
          child: Center(
            child: Container(
              width: 440,
              padding: const EdgeInsets.all(NJSpacing.xxl),
              decoration: BoxDecoration(
                color: NJColors.surfaceContainerLowest,
                borderRadius: BorderRadius.circular(NJRadius.xl),
                boxShadow: [
                  BoxShadow(
                    color: Colors.black.withOpacity(0.2),
                    blurRadius: 24,
                    offset: const Offset(0, 8),
                  ),
                ],
              ),
              child: Column(
                mainAxisSize: MainAxisSize.min,
                children: [
                  // Icon
                  Container(
                    width: 64, height: 64,
                    decoration: BoxDecoration(
                      color: NJColors.primaryFixed,
                      shape: BoxShape.circle,
                    ),
                    child: Icon(
                      _iconFor(event.test),
                      size: 30,
                      color: NJColors.primary,
                    ),
                  ),

                  const SizedBox(height: NJSpacing.lg),

                  // Test label
                  Text(
                    event.label,
                    style: NJText.headlineMd(),
                    textAlign: TextAlign.center,
                  ),

                  const SizedBox(height: NJSpacing.sm),

                  // Instruction
                  Text(
                    event.instruction,
                    style: NJText.bodyMd(),
                    textAlign: TextAlign.center,
                  ),

                  const SizedBox(height: NJSpacing.xxl),

                  // Buttons
                  Row(
                    children: [
                      // Fail
                      Expanded(
                        child: OutlinedButton.icon(
                          onPressed: () => onAcknowledge(false),
                          icon: const Icon(Icons.close_rounded, size: 18),
                          label: const Text('Not Working'),
                          style: OutlinedButton.styleFrom(
                            foregroundColor: NJColors.error,
                            side: BorderSide(
                                color: NJColors.error.withOpacity(0.5)),
                            padding: const EdgeInsets.symmetric(
                                vertical: NJSpacing.md),
                            shape: const StadiumBorder(),
                          ),
                        ),
                      ),

                      const SizedBox(width: NJSpacing.md),

                      // Pass
                      Expanded(
                        child: FilledButton.icon(
                          onPressed: () => onAcknowledge(true),
                          icon: const Icon(Icons.check_rounded, size: 18),
                          label: const Text('Done / Working'),
                          style: FilledButton.styleFrom(
                            backgroundColor: const Color(0xFF2E7D32),
                            foregroundColor: Colors.white,
                            padding: const EdgeInsets.symmetric(
                                vertical: NJSpacing.md),
                            shape: const StadiumBorder(),
                          ),
                        ),
                      ),
                    ],
                  ),
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }

  IconData _iconFor(String testId) {
    if (testId.contains('LID'))       return Icons.laptop_outlined;
    if (testId.contains('HDMI') ||
        testId.contains('VGA'))       return Icons.monitor_outlined;
    if (testId.contains('USB'))       return Icons.usb_outlined;
    if (testId.contains('Bluetooth')) return Icons.bluetooth_outlined;
    return Icons.touch_app_outlined;
  }
}
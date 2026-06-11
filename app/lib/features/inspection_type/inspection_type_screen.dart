// ─────────────────────────────────────────────────────────────────
// inspection_type_screen.dart
//
// Shown after login. Loads inspection types from ERP for this user,
// displays them as selectable cards, runs battery pre-flight on
// selection, then navigates to /diagnostic.
//
// Logic wiring (zero changes to inspection_type_service.dart):
//   init       → InspectionTypeService.fetchTypes(username)
//   on tap     → fetchMinCharge(type) + getCurrentBatteryPct()
//   battery ok → saveChosenType(type) → /diagnostic
//   single type → auto-selects after 400ms (matches C++ behaviour)
// ─────────────────────────────────────────────────────────────────

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../core/nj_theme.dart';
import '../auth/auth_service.dart';
import 'inspection_type_service.dart';

// ═══════════════════════════════════════════════════════════════════
// STATE
// ═══════════════════════════════════════════════════════════════════

enum _Phase { loading, loaded, batteryCheck, batteryLow, error }

class _InspState {
  final _Phase phase;
  final List<String> types;
  final String? selectedType;  // which card is in batteryCheck
  final int currentBattery;
  final int minCharge;
  final String? errorMessage;

  const _InspState({
    this.phase = _Phase.loading,
    this.types = const [],
    this.selectedType,
    this.currentBattery = 0,
    this.minCharge = 0,
    this.errorMessage,
  });

  _InspState copyWith({
    _Phase? phase,
    List<String>? types,
    String? selectedType,
    int? currentBattery,
    int? minCharge,
    String? errorMessage,
  }) =>
      _InspState(
        phase: phase ?? this.phase,
        types: types ?? this.types,
        selectedType: selectedType ?? this.selectedType,
        currentBattery: currentBattery ?? this.currentBattery,
        minCharge: minCharge ?? this.minCharge,
        errorMessage: errorMessage ?? this.errorMessage,
      );
}

class _InspNotifier extends StateNotifier<_InspState> {
  _InspNotifier() : super(const _InspState()) {
    _load();
  }

  Future<void> _load() async {
    final session = AuthService.instance.session;
    if (session == null) {
      state = const _InspState(
        phase: _Phase.error,
        errorMessage: 'Session expired. Please log in again.',
      );
      return;
    }

    debugPrint('[INSP] Fetching inspection types for ${session.username}');
    final types = await InspectionTypeService.instance.fetchTypes(session.username);
    debugPrint('[INSP] Types received: $types');

    if (types.isEmpty) {
      state = const _InspState(
        phase: _Phase.error,
        errorMessage:
            'No inspection types mapped to your role.\nContact your TL / ML.',
      );
      return;
    }

    state = _InspState(phase: _Phase.loaded, types: types);
  }

  // Returns true on success (caller handles navigation), false if blocked.
  Future<bool> selectType(String type) async {
    debugPrint('[INSP] Selected: $type — running battery check...');
    state = state.copyWith(phase: _Phase.batteryCheck, selectedType: type);

    final minCharge =
        await InspectionTypeService.instance.fetchMinCharge(type);
    debugPrint('[INSP] minCharge=$minCharge');

    if (minCharge > 0) {
      final current =
          await InspectionTypeService.instance.getCurrentBatteryPct();
      debugPrint('[INSP] currentBattery=$current');

      if (current < minCharge) {
        debugPrint('[INSP] Battery too low — blocking');
        state = state.copyWith(
          phase: _Phase.batteryLow,
          currentBattery: current,
          minCharge: minCharge,
        );
        return false;
      }
    }

    debugPrint('[INSP] Battery ok — saving type and proceeding');
    await InspectionTypeService.instance.saveChosenType(type);
    // Reset state so the card isn't stuck in loading if user navigates back
    state = state.copyWith(phase: _Phase.loaded, selectedType: null);
    return true;
  }

  void dismissBatteryWarning() {
    state = state.copyWith(
      phase: _Phase.loaded,
      selectedType: null,
    );
  }

  void retry() => _load();
}

final _inspProvider =
    StateNotifierProvider.autoDispose<_InspNotifier, _InspState>(
  (ref) => _InspNotifier(),
);

// ═══════════════════════════════════════════════════════════════════
// SCREEN
// ═══════════════════════════════════════════════════════════════════

class InspectionTypeScreen extends ConsumerStatefulWidget {
  const InspectionTypeScreen({super.key});

  @override
  ConsumerState<InspectionTypeScreen> createState() =>
      _InspectionTypeScreenState();
}

class _InspectionTypeScreenState extends ConsumerState<InspectionTypeScreen> {
  @override
  Widget build(BuildContext context) {
    final st = ref.watch(_inspProvider);

    // Auto-select when single type loads
    ref.listen<_InspState>(_inspProvider, (prev, next) {
      if (prev?.phase == _Phase.loading &&
          next.phase == _Phase.loaded &&
          next.types.length == 1) {
        debugPrint('[INSP] Single type — auto-selecting ${next.types.first}');
        Future.delayed(const Duration(milliseconds: 400), () {
          if (mounted) _doSelect(next.types.first);
        });
      }
    });

    return Scaffold(
      body: Row(
        children: [
          // ── Left: brand + user info
          SizedBox(
            width: 300,
            child: _SidePanel(session: AuthService.instance.session),
          ),

          // ── Right: type selection
          Expanded(
            child: _MainPanel(
              state: st,
              onSelect: _doSelect,
              onDismissBattery: () =>
                  ref.read(_inspProvider.notifier).dismissBatteryWarning(),
              onRetry: () => ref.read(_inspProvider.notifier).retry(),
            ),
          ),
        ],
      ),
    );
  }

  Future<void> _doSelect(String type) async {
    final ok = await ref.read(_inspProvider.notifier).selectType(type);
    if (ok && mounted) {
      final session = AuthService.instance.session!;
      Navigator.of(context).pushReplacementNamed(
        '/diagnostic',
        arguments: {
          'username': session.username,
          'password': session.password,
          'inspectionType': type,
        },
      );
    }
  }
}

// ═══════════════════════════════════════════════════════════════════
// LEFT PANEL
// ═══════════════════════════════════════════════════════════════════

class _SidePanel extends StatelessWidget {
  final NJSession? session;
  const _SidePanel({required this.session});

  @override
  Widget build(BuildContext context) {
    return Stack(
      children: [
        // Blue background
        Positioned.fill(child: ColoredBox(color: NJColors.primary)),

        // Radial overlays
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

        // Content
        Padding(
          padding: const EdgeInsets.all(NJSpacing.xl),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              const SizedBox(height: NJSpacing.lg),

              // Logo
              _NJLogo(),

              const SizedBox(height: NJSpacing.xxl),

              // Greeting
              Text(
                _greeting(),
                style: NJText.bodyMd(
                  color: NJColors.onPrimary.withOpacity(0.6),
                ),
              ),
              const SizedBox(height: NJSpacing.xs),
              Text(
                session?.username ?? '—',
                style: NJText.headlineMd(color: NJColors.onPrimary),
                maxLines: 2,
                overflow: TextOverflow.ellipsis,
              ),

              const SizedBox(height: NJSpacing.md),

              // Department badge
              if (session?.department.isNotEmpty == true)
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
                      Icon(
                        Icons.badge_outlined,
                        size: 14,
                        color: NJColors.onPrimary.withOpacity(0.7),
                      ),
                      const SizedBox(width: NJSpacing.xs),
                      Text(
                        session!.department,
                        style: NJText.labelSm(
                          color: NJColors.onPrimary.withOpacity(0.9),
                        ),
                      ),
                    ],
                  ),
                ),

              const Spacer(),

              // Footer hint
              Text(
                'Select an inspection type\nto begin the diagnostic session.',
                style: NJText.bodySm(
                  color: NJColors.onPrimary.withOpacity(0.5),
                ),
              ),
            ],
          ),
        ),

        // Version badge
        Positioned(
          left: NJSpacing.xl,
          bottom: NJSpacing.xl,
          child: Text(
            'v${kAppVersion.toStringAsFixed(1)}',
            style: NJText.labelSm(
              color: NJColors.onPrimary.withOpacity(0.4),
            ),
          ),
        ),
      ],
    );
  }

  String _greeting() {
    final h = DateTime.now().hour;
    if (h < 12) return 'Good morning';
    if (h < 17) return 'Good afternoon';
    return 'Good evening';
  }
}

class _NJLogo extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(
        horizontal: NJSpacing.md,
        vertical: NJSpacing.sm,
      ),
      decoration: BoxDecoration(
        color: Colors.white.withOpacity(0.14),
        borderRadius: BorderRadius.circular(NJRadius.md),
      ),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          Container(
            width: 36,
            height: 36,
            decoration: BoxDecoration(
              color: NJColors.onPrimary,
              borderRadius: BorderRadius.circular(NJRadius.xs),
            ),
            child: Center(
              child: Text('NJ', style: NJText.labelLg(color: NJColors.primary)),
            ),
          ),
          const SizedBox(width: NJSpacing.sm),
          Text(
            'NewJaisa',
            style: NJText.titleLg(color: NJColors.onPrimary)
                .copyWith(fontWeight: FontWeight.w700),
          ),
        ],
      ),
    );
  }
}

// ═══════════════════════════════════════════════════════════════════
// RIGHT PANEL
// ═══════════════════════════════════════════════════════════════════

class _MainPanel extends StatelessWidget {
  final _InspState state;
  final ValueChanged<String> onSelect;
  final VoidCallback onDismissBattery;
  final VoidCallback onRetry;

  const _MainPanel({
    required this.state,
    required this.onSelect,
    required this.onDismissBattery,
    required this.onRetry,
  });

  @override
  Widget build(BuildContext context) {
    return Container(
      color: NJColors.surfaceContainerLowest,
      child: Center(
        child: SizedBox(
          width: 520,
          child: Padding(
            padding: const EdgeInsets.symmetric(vertical: NJSpacing.xxl),
            child: Column(
              mainAxisSize: MainAxisSize.min,
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                // Header
                Text('Inspection type', style: NJText.headlineLg()),
                const SizedBox(height: NJSpacing.xs),
                Text(
                  'Choose the inspection type for this session.',
                  style: NJText.bodyMd(),
                ),

                const SizedBox(height: NJSpacing.xl),

                // Battery low warning
                if (state.phase == _Phase.batteryLow) ...[
                  _BatteryBanner(
                    current: state.currentBattery,
                    required_: state.minCharge,
                    type: state.selectedType ?? '',
                    onDismiss: onDismissBattery,
                  ),
                  const SizedBox(height: NJSpacing.md),
                ],

                // Card list
                _buildCards(),
              ],
            ),
          ),
        ),
      ),
    );
  }

  Widget _buildCards() {
    switch (state.phase) {
      case _Phase.loading:
        return Column(
          children: List.generate(
            3,
            (_) => const Padding(
              padding: EdgeInsets.only(bottom: NJSpacing.md),
              child: _SkeletonCard(),
            ),
          ),
        );

      case _Phase.error:
        return _ErrorState(
          message: state.errorMessage ?? 'Something went wrong.',
          onRetry: onRetry,
        );

      case _Phase.loaded:
      case _Phase.batteryCheck:
      case _Phase.batteryLow:
        return Column(
          children: state.types.map((type) {
            final isChecking =
                state.phase == _Phase.batteryCheck &&
                state.selectedType == type;
            return Padding(
              padding: const EdgeInsets.only(bottom: NJSpacing.md),
              child: _TypeCard(
                type: type,
                isChecking: isChecking,
                isDisabled: state.phase == _Phase.batteryCheck &&
                    state.selectedType != type,
                onTap: () => onSelect(type),
              ),
            );
          }).toList(),
        );
    }
  }
}

// ═══════════════════════════════════════════════════════════════════
// TYPE CARD
// ═══════════════════════════════════════════════════════════════════

class _TypeCard extends StatefulWidget {
  final String type;
  final bool isChecking;
  final bool isDisabled;
  final VoidCallback onTap;

  const _TypeCard({
    required this.type,
    required this.isChecking,
    required this.isDisabled,
    required this.onTap,
  });

  @override
  State<_TypeCard> createState() => _TypeCardState();
}

class _TypeCardState extends State<_TypeCard> {
  bool _hovered = false;

  @override
  Widget build(BuildContext context) {
    return MouseRegion(
      onEnter: (_) => setState(() => _hovered = true),
      onExit: (_) => setState(() => _hovered = false),
      child: GestureDetector(
        onTap: widget.isDisabled || widget.isChecking ? null : widget.onTap,
        child: AnimatedContainer(
          duration: const Duration(milliseconds: 150),
          height: 80,
          decoration: BoxDecoration(
            color: _hovered && !widget.isDisabled
                ? NJColors.primaryFixed.withOpacity(0.3)
                : NJColors.surfaceContainerLowest,
            borderRadius: BorderRadius.circular(NJRadius.lg),
            border: Border.all(
              color: widget.isChecking
                  ? NJColors.primary
                  : _hovered && !widget.isDisabled
                      ? NJColors.primaryContainer
                      : NJColors.outlineVariant,
              width: widget.isChecking ? 2.0 : 1.0,
            ),
          ),
          child: Padding(
            padding: const EdgeInsets.symmetric(horizontal: NJSpacing.lg),
            child: Row(
              children: [
                // Icon circle
                Container(
                  width: 44,
                  height: 44,
                  decoration: BoxDecoration(
                    color: widget.isDisabled
                        ? NJColors.surfaceContainer
                        : NJColors.primaryFixed,
                    shape: BoxShape.circle,
                  ),
                  child: Icon(
                    _iconFor(widget.type),
                    size: 20,
                    color: widget.isDisabled
                        ? NJColors.outline
                        : NJColors.primary,
                  ),
                ),

                const SizedBox(width: NJSpacing.md),

                // Type name
                Expanded(
                  child: Text(
                    widget.type,
                    style: NJText.titleMd(
                      color: widget.isDisabled
                          ? NJColors.outline
                          : NJColors.onSurface,
                    ),
                  ),
                ),

                // Right side: spinner or chevron
                if (widget.isChecking)
                  const SizedBox(
                    width: 20,
                    height: 20,
                    child: CircularProgressIndicator(
                      strokeWidth: 2,
                      color: NJColors.primary,
                    ),
                  )
                else
                  Icon(
                    Icons.chevron_right_rounded,
                    color: widget.isDisabled
                        ? NJColors.outlineVariant
                        : NJColors.primary,
                    size: 22,
                  ),
              ],
            ),
          ),
        ),
      ),
    );
  }

  IconData _iconFor(String type) {
    final t = type.toLowerCase();
    if (t.contains('iqc') || t.contains('core')) return Icons.laptop_outlined;
    if (t.contains('mdt') || t.contains('post')) return Icons.build_circle_outlined;
    if (t.contains('reliab')) return Icons.speed_outlined;
    if (t.contains('battery')) return Icons.battery_charging_full_outlined;
    if (t.contains('storage') || t.contains('hdd')) return Icons.storage_outlined;
    if (t.contains('network') || t.contains('wifi')) return Icons.wifi_outlined;
    return Icons.assignment_outlined;
  }
}

// ═══════════════════════════════════════════════════════════════════
// SUB-WIDGETS
// ═══════════════════════════════════════════════════════════════════

class _BatteryBanner extends StatelessWidget {
  final int current;
  final int required_;
  final String type;
  final VoidCallback onDismiss;

  const _BatteryBanner({
    required this.current,
    required this.required_,
    required this.type,
    required this.onDismiss,
  });

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(
        horizontal: NJSpacing.md,
        vertical: NJSpacing.sm + 2,
      ),
      decoration: BoxDecoration(
        color: NJColors.warningContainer,
        borderRadius: BorderRadius.circular(NJRadius.sm),
        border: Border.all(color: NJColors.warning.withOpacity(0.3)),
      ),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Padding(
            padding: EdgeInsets.only(top: 2),
            child: Icon(Icons.battery_alert_rounded,
                size: 18, color: NJColors.warning),
          ),
          const SizedBox(width: NJSpacing.sm),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  'Battery too low for $type',
                  style: NJText.labelMd(color: NJColors.warning),
                ),
                const SizedBox(height: 2),
                Text(
                  'Current charge: $current%. Required: $required_%. '
                  'Please connect the charger and try again.',
                  style: NJText.bodySm(color: NJColors.warning),
                ),
              ],
            ),
          ),
          GestureDetector(
            onTap: onDismiss,
            child: const Icon(Icons.close_rounded,
                size: 16, color: NJColors.warning),
          ),
        ],
      ),
    );
  }
}

class _SkeletonCard extends StatelessWidget {
  const _SkeletonCard();

  @override
  Widget build(BuildContext context) {
    return Container(
      height: 80,
      decoration: BoxDecoration(
        color: NJColors.surfaceContainerLow,
        borderRadius: BorderRadius.circular(NJRadius.lg),
        border: Border.all(color: NJColors.outlineVariant),
      ),
      child: Padding(
        padding: const EdgeInsets.symmetric(horizontal: NJSpacing.lg),
        child: Row(
          children: [
            Container(
              width: 44,
              height: 44,
              decoration: const BoxDecoration(
                color: NJColors.surfaceContainer,
                shape: BoxShape.circle,
              ),
            ),
            const SizedBox(width: NJSpacing.md),
            Column(
              mainAxisAlignment: MainAxisAlignment.center,
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Container(
                  width: 180,
                  height: 14,
                  decoration: BoxDecoration(
                    color: NJColors.surfaceContainer,
                    borderRadius: BorderRadius.circular(NJRadius.xs),
                  ),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }
}

class _ErrorState extends StatelessWidget {
  final String message;
  final VoidCallback onRetry;

  const _ErrorState({required this.message, required this.onRetry});

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.all(NJSpacing.xl),
      decoration: BoxDecoration(
        color: NJColors.errorContainer,
        borderRadius: BorderRadius.circular(NJRadius.lg),
        border: Border.all(color: NJColors.error.withOpacity(0.3)),
      ),
      child: Column(
        children: [
          const Icon(Icons.error_outline_rounded,
              size: 32, color: NJColors.onErrorContainer),
          const SizedBox(height: NJSpacing.md),
          Text(
            message,
            textAlign: TextAlign.center,
            style: NJText.bodyMd(color: NJColors.onErrorContainer),
          ),
          const SizedBox(height: NJSpacing.lg),
          OutlinedButton(
            onPressed: onRetry,
            style: OutlinedButton.styleFrom(
              foregroundColor: NJColors.onErrorContainer,
              side: BorderSide(color: NJColors.error.withOpacity(0.4)),
              shape: const StadiumBorder(),
            ),
            child: Text('Retry', style: NJText.labelLg(color: NJColors.onErrorContainer)),
          ),
        ],
      ),
    );
  }
}
// ─────────────────────────────────────────────────────────────────
// main.dart — NJ Diagnostic App entry point
// ─────────────────────────────────────────────────────────────────

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:window_manager/window_manager.dart';

import 'core/nj_theme.dart';
import 'core/njqca_runner.dart';
import 'features/auth/login_screen.dart';
import 'features/inspection_type/inspection_type_screen.dart';
import 'features/diagnostic/diagnostic_screen.dart';
import 'features/results/results_screen.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();

  // Window setup
  await windowManager.ensureInitialized();
  await windowManager.waitUntilReadyToShow(
    const WindowOptions(
      minimumSize: Size(1100, 700),
      size:        Size(1280, 800),
      center:      true,
      title:       'NJ Diagnostic App',
      backgroundColor: Colors.transparent,
      titleBarStyle: TitleBarStyle.normal,
    ),
    () async => await windowManager.show(),
  );

  // Check if this is a post-restart resume
  final restartState = await NjqcaRunner.checkRestartState();

  runApp(ProviderScope(
    overrides: [
      if (restartState != null)
        restartStateProvider.overrideWithValue(restartState),
    ],
    child: const NJDiagnosticApp(),
  ));
}

// ── Restart state provider (null = normal startup) ────────────────
final restartStateProvider = Provider<RestartState?>((ref) => null);

// ── App ───────────────────────────────────────────────────────────
class NJDiagnosticApp extends ConsumerWidget {
  const NJDiagnosticApp({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final restartState = ref.watch(restartStateProvider);

    return MaterialApp(
      title:                    'NJ Diagnostic App',
      debugShowCheckedModeBanner: false,
      theme:                    njTheme(),
      // If we resumed from a restart, go straight to diagnostic screen
      initialRoute: restartState != null ? '/diagnostic' : '/login',
      routes: {
        '/login':           (_) => const LoginScreen(),
        '/inspection-type': (_) => const InspectionTypeScreen(),
        '/diagnostic':      (_) => const DiagnosticScreen(),
        '/results':         (_) => const ResultsScreen(),
        // Interactive screens registered as named routes
        // so DiagnosticScreen can push them as overlays
      },
    );
  }
}

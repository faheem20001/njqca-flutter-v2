// ─────────────────────────────────────────────────────────────────
// njqca_runner.dart
// Launches NJQCA.exe and streams NjEvent objects.
//
// Before launch, writes two files NJQCA.exe reads on startup:
//   Temp_Data\flutter_auth.txt             → username\npassword
//   Temp_Data\flutter_inspection_type.txt  → inspection type string
//   Temp_Data\njqca_inspection_type.txt    → same (for MyScript.ps1)
//
// NJQCA.exe emits JSON lines to stdout.
// This class reads them, parses them, and exposes a Stream<NjEvent>.
//
// Flutter never calls NJQCA.exe directly — always through this class.
// ─────────────────────────────────────────────────────────────────

import 'dart:async';
import 'dart:convert';
import 'dart:io';

import 'event_model.dart';

export 'event_model.dart';

class NjqcaRunner {
  // ── Root directory ─────────────────────────────────────────────
  // DEV:  hardcoded to C:\\WindowsNJQCA so Flutter debug build
  //       can reach the compiled NJQCA.exe and its db\\ folder.
  // PROD: set _devMode = false when Flutter .exe is deployed
  //       alongside NJQCA.exe in the same folder.
  static const bool   _devMode = true;
  static const String _devRoot = r'C:\NJ\c_source_code';

  static String get _rootDir {
    if (_devMode) return _devRoot;
    final exePath = Platform.resolvedExecutable;
    return File(exePath).parent.path;
  }

  static String get _njqcaExe => '$_rootDir\\NJQCA.exe';
  static String get _tempDir  => '$_rootDir\\Temp_Data';
  static String get _ackFile  => '$_tempDir\\interactive_ack.txt';

  // Active process — kept so we can kill it if needed
  Process? _process;
  bool _killed = false;

  // ── Launch NJQCA.exe and return an event stream ────────────────
  Stream<NjEvent> launch({
    required String username,
    required String password,
    required String inspectionType,
  }) async* {
    _killed = false;

    // Write credential and inspection type files before launching.
    // NJQCA.exe reads these on startup instead of showing console prompts.
    await _writeFlutterAuth(username, password);
    await _writeInspectionType(inspectionType);

    // Launch NJQCA.exe — no positional args needed (reads from files)
    _process = await Process.start(
      _njqcaExe,
      [],
      workingDirectory: _rootDir,
      runInShell: false,
    );

    debugPrint('[NjqcaRunner] Launched: $_njqcaExe');
    debugPrint('[NjqcaRunner] User: $username | Type: $inspectionType');

    // Merge stdout + stderr into one line stream.
    // allowMalformed: true prevents crashes when incompatible 32-bit exes
    // emit Windows codepage (non-UTF-8) error bytes into stdout/stderr.
    final stdoutLines = _process!.stdout
        .transform(const Utf8Decoder(allowMalformed: true))
        .transform(const LineSplitter());

    final stderrLines = _process!.stderr
        .transform(const Utf8Decoder(allowMalformed: true))
        .transform(const LineSplitter())
        .map((l) => '[ERR] $l');

    final controller = StreamController<String>();
    stdoutLines.listen(controller.add, onDone: () {});
    stderrLines.listen(controller.add, onDone: () => controller.close());

    // Buffer for assembling multi-line JSON from NJQCA.exe stdout.
    // Brace depth is only counted while inside a JSON block so that
    // non-JSON lines containing { or } (paths, table borders, etc.)
    // cannot corrupt the counter.
    final StringBuffer jsonBuffer = StringBuffer();
    int braceDepth = 0;
    bool inJson = false;

    await for (final line in controller.stream) {
      if (_killed) break;

      final trimmed = line.trim();
      debugPrint('[NJQCA.exe] $trimmed');

      if (trimmed.startsWith('{') || inJson) {
        inJson = true;
        jsonBuffer.writeln(trimmed);

        // Count braces only while collecting a JSON object
        for (final ch in trimmed.runes) {
          if (ch == '{'.codeUnitAt(0)) braceDepth++;
          if (ch == '}'.codeUnitAt(0)) braceDepth--;
        }

        if (braceDepth == 0) {
          // Complete JSON object assembled
          final raw = jsonBuffer.toString().trim();
          jsonBuffer.clear();
          inJson = false;

          try {
            final decoded = jsonDecode(raw) as Map<String, dynamic>;
            yield NjEvent.fromJson(decoded);
          } catch (e) {
            debugPrint('[NjqcaRunner] Failed to parse JSON: $e');
            debugPrint('[NjqcaRunner] Raw was: $raw');
          }
        }
      }
    }

    await _process!.exitCode;
    debugPrint('[NjqcaRunner] NJQCA.exe exited');
  }

  // ── Interactive test handshake ─────────────────────────────────
  // Flutter calls this when the user completes an interactive test.
  // C++ polls Temp_Data\interactive_ack.txt and reads "pass" or "fail".
  Future<void> acknowledgeInteractive(String testId, {bool passed = true}) async {
    final ackFile = File(_ackFile);
    await ackFile.writeAsString(passed ? 'pass' : 'fail');
    debugPrint('[NjqcaRunner] ack written: ${passed ? "pass" : "fail"} for $testId');
  }

  // ── Kill NJQCA.exe (e.g. user cancels) ────────────────────────
  void kill() {
    _killed = true;
    _process?.kill();
    debugPrint('[NjqcaRunner] NJQCA.exe killed by Flutter');
  }

  // ── Restart resume state ───────────────────────────────────────
  // Called on app startup to check if this is a post-restart resume.
  static Future<RestartState?> checkRestartState() async {
    final f = File('$_rootDir\\Temp_Data\\flutter_restart_state.json');
    if (!await f.exists()) return null;
    try {
      final json = jsonDecode(await f.readAsString()) as Map<String, dynamic>;
      await f.delete(); // consume it
      return RestartState(
        username:       json['username']       as String? ?? '',
        password:       json['password']       as String? ?? '',
        inspectionType: json['inspectionType'] as String? ?? '',
        restartNumber:  (json['restartNumber'] as num?)?.toInt() ?? 1,
      );
    } catch (_) {
      return null;
    }
  }

  // ── Private helpers ────────────────────────────────────────────

  /// Writes credentials for NJQCA.exe to pick up on startup.
  /// C++ reads Temp_Data\flutter_auth.txt (username on line 1, password on line 2)
  /// then deletes it. USB scan only runs as fallback if this file is absent.
  Future<void> _writeFlutterAuth(String username, String password) async {
    await Directory(_tempDir).create(recursive: true);
    await File('$_tempDir\\flutter_auth.txt')
        .writeAsString('$username\n$password');
    debugPrint('[NjqcaRunner] flutter_auth.txt written for $username');
  }

  /// Writes the selected inspection type to two files:
  ///   flutter_inspection_type.txt — C++ reads this to bypass console selection
  ///   njqca_inspection_type.txt   — MyScript.ps1 reads this for restart resume
  Future<void> _writeInspectionType(String type) async {
    await Directory(_tempDir).create(recursive: true);
    await File('$_tempDir\\flutter_inspection_type.txt').writeAsString(type);
    await File('$_tempDir\\njqca_inspection_type.txt').writeAsString(type);
    debugPrint('[NjqcaRunner] inspection type files written: $type');
  }

  void debugPrint(String msg) {
    // ignore: avoid_print
    print(msg);
  }
}

// ── RestartState ───────────────────────────────────────────────────
class RestartState {
  final String username;
  final String password;
  final String inspectionType;
  final int restartNumber;

  const RestartState({
    required this.username,
    required this.password,
    required this.inspectionType,
    required this.restartNumber,
  });
}
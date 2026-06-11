// ─────────────────────────────────────────────────────────────────
// auth_service.dart
// Handles: USB scan → ERP login → SetToken → version check
// After this succeeds → user goes to inspection_type_screen
// ─────────────────────────────────────────────────────────────────

import 'dart:io';
import 'package:dio/dio.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter_secure_storage/flutter_secure_storage.dart';

import '../../core/erp_client.dart';

const double kAppVersion = 8.8;

// ── Session model ─────────────────────────────────────────────────
class NJSession {
  final String username;
  final String password;   // kept for NJQCA.exe launch args
  final String department;
  final String apiKey;
  final String apiSecret;

  const NJSession({
    required this.username,
    required this.password,
    required this.department,
    required this.apiKey,
    required this.apiSecret,
  });
}

// ── Login result ──────────────────────────────────────────────────
enum LoginStatus { success, networkError, wrongCredentials, serverError }

class LoginResult {
  final LoginStatus status;
  final NJSession?  session;
  final double?     serverVersion;
  final bool        needsUpdate;
  final String?     errorMessage;

  const LoginResult._({
    required this.status,
    this.session,
    this.serverVersion,
    this.needsUpdate = false,
    this.errorMessage,
  });

  factory LoginResult.success({
    required NJSession session,
    double? serverVersion,
    bool needsUpdate = false,
  }) => LoginResult._(
        status: LoginStatus.success,
        session: session,
        serverVersion: serverVersion,
        needsUpdate: needsUpdate,
      );

  factory LoginResult.networkError() => const LoginResult._(
        status: LoginStatus.networkError,
        errorMessage: 'Cannot connect to NJ ERP. Check your network.',
      );

  factory LoginResult.wrongCredentials() => const LoginResult._(
        status: LoginStatus.wrongCredentials,
        errorMessage: 'Incorrect username or password.',
      );

  factory LoginResult.serverError(String msg) =>
      LoginResult._(status: LoginStatus.serverError, errorMessage: msg);

  bool get isSuccess      => status == LoginStatus.success;
  bool get isNetworkError => status == LoginStatus.networkError;
}

// ── AuthService ───────────────────────────────────────────────────
class AuthService {
  AuthService._();
  static final instance = AuthService._();

  final _storage = const FlutterSecureStorage();
  final _client  = ErpClient.instance;

  NJSession? _session;
  NJSession? get session => _session;
  bool get isLoggedIn => _session != null;

  // ── USB credential scan ───────────────────────────────────────
  Future<({String username, String password})?> readUsbCredentials() async {
    debugPrint('[AUTH] USB scan: starting PowerShell drive enumeration');
    try {
      final raw = await Process.run('powershell',
          ['-NoProfile', '-Command',
           r'Get-WmiObject Win32_LogicalDisk | Select-Object -ExpandProperty DeviceID']);
      final drives = (raw.stdout as String)
          .split('\n').map((l) => l.trim()).where((l) => l.isNotEmpty).toList();
      debugPrint('[AUTH] USB scan: drives found = ${drives.join(', ')}');

      for (final drive in drives) {
        debugPrint('[AUTH] USB scan: checking $drive\\NJQCApassword.txt');
        final f = File('$drive\\NJQCApassword.txt');
        if (await f.exists()) {
          debugPrint('[AUTH] USB scan: file exists at $drive');
          final lines = await f.readAsLines();
          debugPrint('[AUTH] USB scan: file has ${lines.length} line(s)');
          if (lines.length >= 2 &&
              lines[0].trim().isNotEmpty && lines[1].trim().isNotEmpty) {
            debugPrint('[AUTH] USB scan: credentials loaded, username=${lines[0].trim()}');
            return (username: lines[0].trim(), password: lines[1].trim());
          } else {
            debugPrint('[AUTH] USB scan: file found but invalid (need ≥2 non-empty lines)');
          }
        }
      }
      debugPrint('[AUTH] USB scan: no NJQCApassword.txt found on any drive');
    } catch (e) {
      debugPrint('[AUTH] USB scan error: $e');
    }
    return null;
  }

  // ── Full login flow ───────────────────────────────────────────
  Future<LoginResult> login({
    required String username,
    required String password,
  }) async {
    // Step 1 — Frappe login (gets cookie)
    debugPrint('[AUTH] Step 1: Frappe login → username=$username');
    String loggedUser;
    try {
      final r = await _client.post(ErpApi.login,
          body: {'usr': username, 'pwd': password});
      final msg = r['message']?.toString() ?? '';
      debugPrint('[AUTH] Step 1: response message="$msg"');
      if (msg != 'Logged In') {
        debugPrint('[AUTH] Step 1: FAIL — message != "Logged In"');
        return LoginResult.wrongCredentials();
      }
      final ur = await _client.post(ErpApi.getLoggedUser);
      loggedUser = ur['message']?.toString() ?? '';
      debugPrint('[AUTH] Step 1: logged_user="$loggedUser"');
      if (loggedUser.isEmpty) {
        debugPrint('[AUTH] Step 1: FAIL — empty logged_user');
        return LoginResult.wrongCredentials();
      }
    } on DioException catch (e) {
      debugPrint('[AUTH] Step 1: FAIL — network error type=${e.type} msg=${e.message}');
      return LoginResult.networkError();
    }

    // Step 2 — SetToken (gets api_key, api_secret, department)
    debugPrint('[AUTH] Step 2: SetToken...');
    NJSession session;
    try {
      final r = await _client.post(ErpApi.setToken,
          body: {'usr': username, 'pwd': password});
      final msg = r['message'] as Map<String, dynamic>? ?? {};
      final apiKey     = msg['api_key']    as String? ?? '';
      final apiSecret  = msg['api_secret'] as String? ?? '';
      final department = msg['department'] as String? ?? '';
      debugPrint('[AUTH] Step 2: api_key=${apiKey.isNotEmpty ? "received (${apiKey.length} chars)" : "MISSING"}');
      debugPrint('[AUTH] Step 2: department="$department"');
      if (apiKey.isNotEmpty) {
        _client.setToken(apiKey, apiSecret);
        debugPrint('[AUTH] Step 2: Authorization header set');
      } else {
        debugPrint('[AUTH] Step 2: WARNING — api_key empty, token NOT set');
      }
      session = NJSession(
        username:   loggedUser,
        password:   password,
        department: department,
        apiKey:     apiKey,
        apiSecret:  apiSecret,
      );
      debugPrint('[AUTH] Step 2: session created for user=$loggedUser');
    } catch (e) {
      debugPrint('[AUTH] Step 2: FAIL — $e');
      return LoginResult.serverError(e.toString());
    }

    // Step 3 — Version check
    debugPrint('[AUTH] Step 3: version check (app=$kAppVersion)...');
    double? serverVersion;
    bool needsUpdate = false;
    try {
      final r = await _client.get(ErpApi.njSettings, query: {
        'filters': '[[\"module\",\"=\",\"NJ Features\"],[\"doc_type\",\"=\",\"NJQCA\"]]',
        'fields':  '[\"parameter\",\"value\"]',
        'limit':   '80',
      });
      for (final e in (r['data'] as List? ?? [])) {
        if ((e as Map)['parameter'] == 'WindowsNJQCA') {
          serverVersion = double.tryParse(e['value']?.toString() ?? '');
          if (serverVersion != null && kAppVersion < serverVersion) {
            needsUpdate = true;
          }
          break;
        }
      }
      debugPrint('[AUTH] Step 3: serverVersion=$serverVersion needsUpdate=$needsUpdate');
    } catch (e) {
      debugPrint('[AUTH] Step 3: version check failed (non-fatal, continuing): $e');
    }

    _session = session;
    debugPrint('[AUTH] Login complete ✓ user=${session.username} dept=${session.department} needsUpdate=$needsUpdate');
    return LoginResult.success(
        session: session,
        serverVersion: serverVersion,
        needsUpdate: needsUpdate);
  }

  // ── Secure storage helpers ────────────────────────────────────
  Future<void> saveCredentials(String u, String p) async {
    await _storage.write(key: 'nj_user', value: u);
    await _storage.write(key: 'nj_pass', value: p);
    debugPrint('[AUTH] saveCredentials: saved for user=$u');
  }

  Future<({String username, String password})?> loadSaved() async {
    final u = await _storage.read(key: 'nj_user');
    final p = await _storage.read(key: 'nj_pass');
    if (u != null && p != null && u.isNotEmpty) {
      debugPrint('[AUTH] loadSaved: found credentials for user=$u');
      return (username: u, password: p);
    }
    debugPrint('[AUTH] loadSaved: no saved credentials');
    return null;
  }

  Future<void> clearSaved() async {
    await _storage.delete(key: 'nj_user');
    await _storage.delete(key: 'nj_pass');
    debugPrint('[AUTH] clearSaved: credentials removed from secure storage');
  }

  void logout() {
    debugPrint('[AUTH] logout: session cleared for user=${_session?.username}');
    _session = null;
    _client.clearAuth();
  }
}
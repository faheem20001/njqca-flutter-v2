// ─────────────────────────────────────────────────────────────────
// inspection_type_service.dart
// Fetches inspection types from ERP for the logged-in user.
// Also handles battery pre-flight check.
// ─────────────────────────────────────────────────────────────────

import 'dart:io';
import 'package:flutter/foundation.dart';
import '../../core/erp_client.dart';

class InspectionTypeService {
  InspectionTypeService._();
  static final instance = InspectionTypeService._();

  final _client = ErpClient.instance;

  // Fetch list of allowed inspection types for this user's role
  // ERP returns: { "message": ["Core IQC - Tech", "Post MDT", "Reliability Test"] }
  Future<List<String>> fetchTypes(String username) async {
  debugPrint('[INSP] fetchTypes: sending user="$username"');  // full email, no strip

  try {
    final r = await _client.post(
      ErpApi.inspectionTypes,
      body: {'user': username},  // pass full email as-is
    );
    debugPrint('[INSP] fetchTypes raw response: $r');

    // ERP returned a server-side error
    if (r.containsKey('exc_type')) {
      debugPrint('[INSP] fetchTypes ERP error: ${r['message']}');
      return [];
    }

    final msg = r['message'];
    if (msg is! List) {
      debugPrint('[INSP] fetchTypes unexpected message type: ${msg.runtimeType} = $msg');
      return [];
    }

    final list = msg.map((e) => e.toString()).toList();
    debugPrint('[INSP] fetchTypes parsed: $list');
    return list;

  } catch (e, st) {
    debugPrint('[INSP] fetchTypes ERROR: $e\n$st');
    return [];
  }
}
  // Fetch minimum battery % for this inspection type
  Future<int> fetchMinCharge(String inspectionType) async {
    try {
      final r = await _client.post(ErpApi.minCharge,
          body: {'inspection_type': inspectionType});
      return (r['message'] as num?)?.toInt() ?? 0;
    } catch (_) {
      return 0;
    }
  }

  // Get current battery % via PowerShell WMI
  Future<int> getCurrentBatteryPct() async {
    try {
      debugPrint('[INSP] getCurrentBatteryPct: running WMI...');
      final r = await Process.run('powershell', [
        '-NoProfile', '-Command',
        r'(Get-WmiObject Win32_Battery).EstimatedChargeRemaining',
      ]).timeout(const Duration(seconds: 5));
      final raw = (r.stdout as String).trim();
      debugPrint('[INSP] getCurrentBatteryPct: raw="$raw"');
      return int.tryParse(raw) ?? 100;
    } catch (_) {
      debugPrint('[INSP] getCurrentBatteryPct: failed/timeout — assuming 100%');
      return 100; // assume ok if WMI fails or times out
    }
  }

  // Write chosen type file — NjqcaRunner also writes it, this is a safety copy
  Future<void> saveChosenType(String type) async {
  final root = File(Platform.resolvedExecutable).parent.path;
  final dir = Directory('$root\\Temp_Data');
  await dir.create(recursive: true);                           // <-- add this back
  await File('${dir.path}\\flutter_inspection_type.txt').writeAsString(type);
  debugPrint('[INSP] saveChosenType: written to ${dir.path}');
}
}
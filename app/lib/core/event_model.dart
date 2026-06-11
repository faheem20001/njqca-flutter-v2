// ─────────────────────────────────────────────────────────────────
// event_model.dart
// Sealed class for every JSON event that NJQCA.exe emits to stdout.
//
// NJQCA.exe emits one JSON line per event, e.g.:
//   {"event":"test_start","test":"HDD_Sentinel_Status","label":"HDD Sentinel"}
//   {"event":"test_done","test":"HDD_Sentinel_Status","status":"pass","score":92}
//
// Flutter reads these via NjqcaRunner and maps them here.
// ─────────────────────────────────────────────────────────────────

sealed class NjEvent {
  const NjEvent();

  /// Parse a raw JSON map from stdout into an NjEvent subclass.
  /// Returns NjUnknownEvent if the event type is not recognised.
  static NjEvent fromJson(Map<String, dynamic> json) {
    final event = json['event'] as String? ?? '';
    return switch (event) {
      'session_start'      => NjSessionStart.fromJson(json),
      'hardware_info'      => NjHardwareInfo.fromJson(json),
      'test_start'         => NjTestStart.fromJson(json),
      'test_progress'      => NjTestProgress.fromJson(json),
      'test_done'          => NjTestDone.fromJson(json),
      'test_skipped'       => NjTestSkipped.fromJson(json),
      'interactive_needed' => NjInteractiveNeeded.fromJson(json),
      'performance_start'  => NjPerformanceStart.fromJson(json),
      'restart_scheduled'  => NjRestartScheduled.fromJson(json),
      'session_complete'   => NjSessionComplete.fromJson(json),
      'error'              => NjErrorEvent.fromJson(json),
      _                    => NjUnknownEvent(raw: json),
    };
  }
}

// ── session_start ─────────────────────────────────────────────────
// {"event":"session_start","user":"QC_042","inspection":"Core IQC - Tech",
//  "serial":"NJ-2024-001","department":"Quality"}
class NjSessionStart extends NjEvent {
  final String user;
  final String inspection;
  final String serial;
  final String department;

  const NjSessionStart({
    required this.user,
    required this.inspection,
    required this.serial,
    required this.department,
  });

  factory NjSessionStart.fromJson(Map<String, dynamic> j) => NjSessionStart(
        user:       j['user']       as String? ?? '',
        inspection: j['inspection'] as String? ?? '',
        serial:     j['serial']     as String? ?? '',
        department: j['department'] as String? ?? '',
      );
}

// ── hardware_info ─────────────────────────────────────────────────
// {"event":"hardware_info","cpu":"Intel i7-1185G7","ram_gb":16,
//  "ssd":"512GB NVMe","os":"Windows 11 Pro","battery_pct":87}
class NjHardwareInfo extends NjEvent {
  final String cpu;
  final int ramGb;
  final String ssd;
  final String os;
  final int batteryPct;

  const NjHardwareInfo({
    required this.cpu,
    required this.ramGb,
    required this.ssd,
    required this.os,
    required this.batteryPct,
  });

  factory NjHardwareInfo.fromJson(Map<String, dynamic> j) => NjHardwareInfo(
        cpu:        j['cpu']         as String? ?? '',
        ramGb:      (j['ram_gb']     as num?)?.toInt() ?? 0,
        ssd:        j['ssd']         as String? ?? '',
        os:         j['os']          as String? ?? '',
        batteryPct: (j['battery_pct'] as num?)?.toInt() ?? 0,
      );
}

// ── test_start ────────────────────────────────────────────────────
// {"event":"test_start","test":"HDD_Sentinel_Status","label":"HDD Sentinel","part":"Storage"}
class NjTestStart extends NjEvent {
  final String test;
  final String label;
  final String part;

  const NjTestStart({
    required this.test,
    required this.label,
    required this.part,
  });

  factory NjTestStart.fromJson(Map<String, dynamic> j) => NjTestStart(
        test:  j['test']  as String? ?? '',
        label: j['label'] as String? ?? '',
        part:  j['part']  as String? ?? '',
      );
}

// ── test_progress ─────────────────────────────────────────────────
// {"event":"test_progress","test":"HDD_Sentinel_Status","msg":"Waiting for report..."}
class NjTestProgress extends NjEvent {
  final String test;
  final String message;

  const NjTestProgress({required this.test, required this.message});

  factory NjTestProgress.fromJson(Map<String, dynamic> j) => NjTestProgress(
        test:    j['test'] as String? ?? '',
        message: j['msg']  as String? ?? '',
      );
}

// ── test_done ─────────────────────────────────────────────────────
// {"event":"test_done","test":"HDD_Sentinel_Status","status":"pass",
//  "score":92,"data":{"health":92,"temp":34,"performance":88}}
class NjTestDone extends NjEvent {
  final String test;
  final String status;   // "pass" | "fail" | "warning"
  final int score;
  final Map<String, dynamic> data;

  bool get isPassed  => status == 'pass';
  bool get isFailed  => status == 'fail';
  bool get isWarning => status == 'warning';

  const NjTestDone({
    required this.test,
    required this.status,
    required this.score,
    this.data = const {},
  });

  factory NjTestDone.fromJson(Map<String, dynamic> j) => NjTestDone(
        test:   j['test']               as String? ?? '',
        status: j['status']             as String? ?? 'fail',
        score:  (j['score'] as num?)?.toInt() ?? 0,
        data:   j['data']  as Map<String, dynamic>? ?? {},
      );
}

// ── test_skipped ──────────────────────────────────────────────────
// {"event":"test_skipped","test":"RAM_speed_status","reason":"cascade"}
class NjTestSkipped extends NjEvent {
  final String test;
  final String reason;   // "cascade" | "not_in_plan"

  const NjTestSkipped({required this.test, required this.reason});

  factory NjTestSkipped.fromJson(Map<String, dynamic> j) => NjTestSkipped(
        test:   j['test']   as String? ?? '',
        reason: j['reason'] as String? ?? '',
      );
}

// ── interactive_needed ────────────────────────────────────────────
// {"event":"interactive_needed","test":"hotkeys","label":"Keyboard Test",
//  "instruction":"Press all highlighted keys on the keyboard"}
// Flutter shows the interactive screen, then writes Temp_Data/interactive_ack.txt
// C++ polls that file and continues
class NjInteractiveNeeded extends NjEvent {
  final String test;
  final String label;
  final String instruction;

  const NjInteractiveNeeded({
    required this.test,
    required this.label,
    required this.instruction,
  });

  factory NjInteractiveNeeded.fromJson(Map<String, dynamic> j) => NjInteractiveNeeded(
        test:        j['test']        as String? ?? '',
        label:       j['label']       as String? ?? '',
        instruction: j['instruction'] as String? ?? '',
      );
}

// ── performance_start ─────────────────────────────────────────────
// {"event":"performance_start","test":"Performance_status"}
// Performance runs in background — no fixed duration, show indeterminate
class NjPerformanceStart extends NjEvent {
  final String test;
  const NjPerformanceStart({required this.test});
  factory NjPerformanceStart.fromJson(Map<String, dynamic> j) =>
      NjPerformanceStart(test: j['test'] as String? ?? '');
}

// ── restart_scheduled ─────────────────────────────────────────────
// {"event":"restart_scheduled","restart_number":1,"max_restarts":3,"delay_seconds":5}
class NjRestartScheduled extends NjEvent {
  final int restartNumber;
  final int maxRestarts;
  final int delaySeconds;

  const NjRestartScheduled({
    required this.restartNumber,
    required this.maxRestarts,
    required this.delaySeconds,
  });

  factory NjRestartScheduled.fromJson(Map<String, dynamic> j) => NjRestartScheduled(
        restartNumber: (j['restart_number'] as num?)?.toInt() ?? 1,
        maxRestarts:   (j['max_restarts']   as num?)?.toInt() ?? 3,
        delaySeconds:  (j['delay_seconds']  as num?)?.toInt() ?? 5,
      );
}

// ── session_complete ──────────────────────────────────────────────
// {"event":"session_complete","erp_url":"https://erp.newjaisa.com/...","score":87}
class NjSessionComplete extends NjEvent {
  final String erpUrl;
  final int score;

  const NjSessionComplete({required this.erpUrl, required this.score});

  factory NjSessionComplete.fromJson(Map<String, dynamic> j) => NjSessionComplete(
        erpUrl: j['erp_url'] as String? ?? '',
        score:  (j['score'] as num?)?.toInt() ?? 0,
      );
}

// ── error ─────────────────────────────────────────────────────────
// {"event":"error","test":"GeekBenchTest","msg":"Timed out after 10 minutes"}
class NjErrorEvent extends NjEvent {
  final String test;
  final String message;

  const NjErrorEvent({required this.test, required this.message});

  factory NjErrorEvent.fromJson(Map<String, dynamic> j) => NjErrorEvent(
        test:    j['test'] as String? ?? '',
        message: j['msg']  as String? ?? '',
      );
}

// ── unknown (safety net) ──────────────────────────────────────────
class NjUnknownEvent extends NjEvent {
  final Map<String, dynamic> raw;
  const NjUnknownEvent({required this.raw});
}

class NjError extends NjEvent {
  final String test;
  final String msg;

  const NjError({
    required this.test,
    required this.msg,
  });

  factory NjError.fromJson(Map<String, dynamic> json) {
    return NjError(
      test: json['test'] as String? ?? '',
      msg: json['msg'] as String? ?? 'Unknown error',
    );
  }
}

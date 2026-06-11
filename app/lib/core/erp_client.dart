// ─────────────────────────────────────────────────────────────────
// erp_client.dart
// Used ONLY for: login, inspection type fetch, version check.
// All other ERP calls (test results, sync) are handled by NJQCA.exe.
// ─────────────────────────────────────────────────────────────────

import 'package:cookie_jar/cookie_jar.dart';
import 'package:dio/dio.dart';
import 'package:dio_cookie_manager/dio_cookie_manager.dart';
import 'package:flutter/foundation.dart';

class ErpApi {
  ErpApi._();
  static const base             = 'https://erp.newjaisa.com';
  static const login            = '/api/method/login';
  static const getLoggedUser    = '/api/method/frappe.auth.get_logged_user';
  static const setToken         = '/api/method/nj_features.nj_features.api.login.login';
  static const njSettings       = '/api/resource/NJ%20Settings';
  static const inspectionTypes  = '/api/method/nj_lib.utils.common_utils.get_inspection_types';
  static const minCharge        = '/api/method/nj_lib.utils.common_utils.fetch_system_min_charge_percentage';
}

class ErpClient {
  ErpClient._();
  static final instance = ErpClient._();

  final _cookieJar = CookieJar();
  late final Dio _dio = _build();

  Dio _build() {
    final dio = Dio(BaseOptions(
      baseUrl:        ErpApi.base,
      connectTimeout: const Duration(seconds: 15),
      receiveTimeout: const Duration(seconds: 30),
      headers:        {'Content-Type': 'application/json', 'Accept': 'application/json'},
      validateStatus: (_) => true,
    ));

    dio.interceptors.add(CookieManager(_cookieJar));

    dio.interceptors.add(InterceptorsWrapper(
      onRequest: (o, h) {
        debugPrint('→ ERP ${o.method} ${o.path}');
        h.next(o);
      },
      onResponse: (r, h) {
        debugPrint('← ERP ${r.statusCode} ${r.requestOptions.path}');
        h.next(r);
      },
      onError: (e, h) {
        debugPrint('✗ ERP ${e.type} ${e.requestOptions.path}: ${e.message}');
        h.next(e);
      },
    ));

    return dio;
  }

  void setToken(String apiKey, String apiSecret) {
    _dio.options.headers['Authorization'] = 'Token $apiKey:$apiSecret';
  }

  void clearAuth() {
    _dio.options.headers.remove('Authorization');
    _cookieJar.deleteAll();
  }

  Future<Map<String, dynamic>> post(String path, {Map<String, dynamic>? body}) async {
    final r = await _dio.post(path, data: body);
    return _unwrap(r);
  }

  Future<Map<String, dynamic>> get(String path, {Map<String, dynamic>? query}) async {
    final r = await _dio.get(path, queryParameters: query);
    return _unwrap(r);
  }

  Map<String, dynamic> _unwrap(Response r) {
    if (r.data is Map<String, dynamic>) return r.data as Map<String, dynamic>;
    return {'_raw': r.data, '_status': r.statusCode};
  }
}

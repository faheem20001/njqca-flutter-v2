// ─────────────────────────────────────────────────────────────────
// login_screen.dart
//
// Two-column layout: left = blue brand panel, right = form.
//
// Logic wiring (zero changes to auth_service.dart needed):
//   initState → loadSaved()          pre-fills fields from secure storage
//   initState → readUsbCredentials() auto-fills + auto-submits if USB found
//   Sign In   → AuthService.login()  3-step: cookie → token → version check
//   success   → pushReplacementNamed('/inspection-type')
//
// Session is stored in AuthService.instance.session after login.
// InspectionTypeScreen reads it from there.
// ─────────────────────────────────────────────────────────────────

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../core/nj_theme.dart';
import 'auth_service.dart';

// ═══════════════════════════════════════════════════════════════════
// STATE
// ═══════════════════════════════════════════════════════════════════

enum _Phase { idle, loading, error, needsUpdate }

class _LoginState {
  final _Phase phase;
  final String? errorMessage;
  final double? serverVersion;

  const _LoginState({
    this.phase = _Phase.idle,
    this.errorMessage,
    this.serverVersion,
  });

  _LoginState copyWith({
    _Phase? phase,
    String? errorMessage,
    double? serverVersion,
  }) =>
      _LoginState(
        phase: phase ?? this.phase,
        errorMessage: errorMessage,
        serverVersion: serverVersion ?? this.serverVersion,
      );
}

class _LoginNotifier extends StateNotifier<_LoginState> {
  _LoginNotifier() : super(const _LoginState());

  /// Runs the full login flow. Returns the session on success, null on failure.
  Future<NJSession?> submit({
    required String username,
    required String password,
    required bool keepSignedIn,
  }) async {
    state = const _LoginState(phase: _Phase.loading);

    final result = await AuthService.instance.login(
      username: username,
      password: password,
    );

    if (!result.isSuccess) {
      state = _LoginState(
        phase: _Phase.error,
        errorMessage: result.errorMessage,
      );
      return null;
    }

    if (keepSignedIn) {
      await AuthService.instance.saveCredentials(username, password);
    } else {
      // Clear any previously saved creds if the user unchecked
      await AuthService.instance.clearSaved();
    }

    if (result.needsUpdate) {
      state = _LoginState(
        phase: _Phase.needsUpdate,
        serverVersion: result.serverVersion,
      );
    } else {
      state = const _LoginState();
    }

    return result.session;
  }

  void clearError() => state = const _LoginState();
}

final _loginProvider =
    StateNotifierProvider.autoDispose<_LoginNotifier, _LoginState>(
  (ref) => _LoginNotifier(),
);

// ═══════════════════════════════════════════════════════════════════
// SCREEN
// ═══════════════════════════════════════════════════════════════════

class LoginScreen extends ConsumerStatefulWidget {
  const LoginScreen({super.key});

  @override
  ConsumerState<LoginScreen> createState() => _LoginScreenState();
}

class _LoginScreenState extends ConsumerState<LoginScreen> {
  final _userCtrl = TextEditingController();
  final _passCtrl = TextEditingController();
  bool _keepSigned = false;
  bool _obscurePass = true;
  bool _internalTab = true; // true = Internal Login, false = Customer Verification

  @override
  void initState() {
    super.initState();
    _initAutoLogin();
  }

  @override
  void dispose() {
    _userCtrl.dispose();
    _passCtrl.dispose();
    super.dispose();
  }

  // ── Auto-login on startup ─────────────────────────────────────
  Future<void> _initAutoLogin() async {
    // 1. Prefill from secure storage (keep-signed-in)
    final saved = await AuthService.instance.loadSaved();
    if (saved != null && mounted) {
      setState(() {
        _userCtrl.text = saved.username;
        _passCtrl.text = saved.password;
        _keepSigned = true;
      });
    }

    // 2. USB scan — overrides saved if found, then auto-submits
    final usb = await AuthService.instance.readUsbCredentials();
    if (usb != null && mounted) {
      setState(() {
        _userCtrl.text = usb.username;
        _passCtrl.text = usb.password;
      });
      // Brief pause so the user can see the fields populate
      await Future.delayed(const Duration(milliseconds: 700));
      if (mounted) _submit();
    }
  }

  // ── Submit ────────────────────────────────────────────────────
  Future<void> _submit() async {
    final u = _userCtrl.text.trim();
    final p = _passCtrl.text.trim();
    if (u.isEmpty || p.isEmpty) return;

    final session = await ref.read(_loginProvider.notifier).submit(
          username: u,
          password: p,
          keepSignedIn: _keepSigned,
        );

    // needsUpdate → still navigate, the banner is shown on the next screen
    // (or handle here if you want to block)
    if (session != null && mounted) {
      Navigator.of(context).pushReplacementNamed('/inspection-type');
    }
  }

  // ── Build ─────────────────────────────────────────────────────
  @override
  Widget build(BuildContext context) {
    final st = ref.watch(_loginProvider);
    final isLoading = st.phase == _Phase.loading;

    return Scaffold(
      body: Row(
        children: [
          // ── Left: blue brand panel (50 %)
          Expanded(
            child: _BrandPanel(),
          ),

          // ── Right: form (50 %)
          Expanded(
            child: _FormPanel(
              userCtrl: _userCtrl,
              passCtrl: _passCtrl,
              keepSigned: _keepSigned,
              obscurePass: _obscurePass,
              internalTab: _internalTab,
              isLoading: isLoading,
              errorMessage:
                  st.phase == _Phase.error ? st.errorMessage : null,
              needsUpdate: st.phase == _Phase.needsUpdate,
              serverVersion: st.serverVersion,
              onToggleTab: (v) => setState(() => _internalTab = v),
              onToggleKeep: (v) =>
                  setState(() => _keepSigned = v ?? false),
              onTogglePass: () =>
                  setState(() => _obscurePass = !_obscurePass),
              onSubmit: isLoading ? null : _submit,
              onClearError: () =>
                  ref.read(_loginProvider.notifier).clearError(),
            ),
          ),
        ],
      ),
    );
  }
}

// ═══════════════════════════════════════════════════════════════════
// LEFT PANEL — BRAND
// ═══════════════════════════════════════════════════════════════════

class _BrandPanel extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Stack(
      children: [
        // Solid primary blue
        Positioned.fill(
          child: ColoredBox(color: NJColors.primary),
        ),

        // Decorative radial overlays (matches Stitch HTML)
        Positioned.fill(
          child: DecoratedBox(
            decoration: BoxDecoration(
              gradient: RadialGradient(
                center: const Alignment(1.0, -1.0),
                radius: 1.2,
                colors: [
                  Colors.white.withOpacity(0.12),
                  Colors.transparent,
                ],
              ),
            ),
          ),
        ),
        Positioned.fill(
          child: DecoratedBox(
            decoration: BoxDecoration(
              gradient: RadialGradient(
                center: const Alignment(-1.0, 1.0),
                radius: 1.2,
                colors: [
                  Colors.white.withOpacity(0.08),
                  Colors.transparent,
                ],
              ),
            ),
          ),
        ),

        // Centered content
        Center(
          child: Padding(
            padding: const EdgeInsets.all(NJSpacing.xxl),
            child: Column(
              mainAxisSize: MainAxisSize.min,
              children: [
                // ── Logo ──────────────────────────────────────
                // TODO: replace with AssetImage('assets/images/nj_logo_white.png')
                //       once the asset is added to pubspec.yaml + assets folder
                Image.asset('assets/images/nj_logo_white.png', height: 64),

                const SizedBox(height: NJSpacing.xl),

                // ── App title ─────────────────────────────────
                Text(
                  'Newjaisa\nDiagnostic App',
                  textAlign: TextAlign.center,
                  style: NJText.displayLg(color: NJColors.onPrimary),
                ),

                const SizedBox(height: NJSpacing.md),

                // ── Tagline ───────────────────────────────────
                Text(
                  'Diagnose. Inspect. Support. Value.',
                  style: NJText.titleLg(
                    color: NJColors.onPrimary.withOpacity(0.65),
                  ),
                ),
              ],
            ),
          ),
        ),

        // Version badge bottom-left
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
}

// ── Inline NJ logo widget ─────────────────────────────────────────
class _NJLogo extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(
        horizontal: NJSpacing.lg,
        vertical: NJSpacing.md,
      ),
      decoration: BoxDecoration(
        color: Colors.white.withOpacity(0.14),
        borderRadius: BorderRadius.circular(NJRadius.lg),
      ),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          // NJ monogram block
          Container(
            width: 44,
            height: 44,
            decoration: BoxDecoration(
              color: NJColors.onPrimary,
              borderRadius: BorderRadius.circular(NJRadius.sm),
            ),
            child: Center(
              child: Text(
                'NJ',
                style: NJText.titleMd(color: NJColors.primary),
              ),
            ),
          ),
          const SizedBox(width: NJSpacing.sm),
          // Word mark
          Text(
            'NewJaisa',
            style: NJText.displayLg(color: NJColors.onPrimary)
                .copyWith(fontSize: 26, fontWeight: FontWeight.w700),
          ),
        ],
      ),
    );
  }
}

// ═══════════════════════════════════════════════════════════════════
// RIGHT PANEL — FORM
// ═══════════════════════════════════════════════════════════════════

class _FormPanel extends StatelessWidget {
  final TextEditingController userCtrl;
  final TextEditingController passCtrl;
  final bool keepSigned;
  final bool obscurePass;
  final bool internalTab;
  final bool isLoading;
  final String? errorMessage;
  final bool needsUpdate;
  final double? serverVersion;
  final ValueChanged<bool> onToggleTab;
  final ValueChanged<bool?> onToggleKeep;
  final VoidCallback onTogglePass;
  final VoidCallback? onSubmit;
  final VoidCallback onClearError;

  const _FormPanel({
    required this.userCtrl,
    required this.passCtrl,
    required this.keepSigned,
    required this.obscurePass,
    required this.internalTab,
    required this.isLoading,
    required this.errorMessage,
    required this.needsUpdate,
    required this.serverVersion,
    required this.onToggleTab,
    required this.onToggleKeep,
    required this.onTogglePass,
    required this.onSubmit,
    required this.onClearError,
  });

  @override
  Widget build(BuildContext context) {
    return Container(
      color: NJColors.surfaceContainerLowest,
      child: Center(
        child: SingleChildScrollView(
          padding: const EdgeInsets.symmetric(
            horizontal: NJSpacing.xxl,
            vertical: NJSpacing.xxl,
          ),
          child: SizedBox(
            width: 400,
            child: Column(
              mainAxisSize: MainAxisSize.min,
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                // ── Segmented toggle ──────────────────────────
                _SegmentedToggle(
                  isInternal: internalTab,
                  onToggle: onToggleTab,
                ),

                const SizedBox(height: NJSpacing.xxl),

                // ── Header ────────────────────────────────────
                Text('Sign in', style: NJText.headlineLg()),
                const SizedBox(height: NJSpacing.xs),
                Text(
                  internalTab
                      ? 'Use your Newjaisa credentials'
                      : 'Enter customer verification code',
                  style: NJText.bodyMd(),
                ),

                const SizedBox(height: NJSpacing.xl),

                // ── Update warning ────────────────────────────
                if (needsUpdate) ...[
                  _UpdateBanner(serverVersion: serverVersion),
                  const SizedBox(height: NJSpacing.md),
                ],

                // ── Error banner ──────────────────────────────
                if (errorMessage != null) ...[
                  _ErrorBanner(
                    message: errorMessage!,
                    onDismiss: onClearError,
                  ),
                  const SizedBox(height: NJSpacing.md),
                ],

                // ── Username field ────────────────────────────
                _NJField(
                  controller: userCtrl,
                  label: 'Username / Email',
                  prefixIcon: Icons.person_outline_rounded,
                  enabled: !isLoading,
                  textInputAction: TextInputAction.next,
                  keyboardType: TextInputType.emailAddress,
                ),

                const SizedBox(height: NJSpacing.md),

                // ── Password field ────────────────────────────
                _NJField(
                  controller: passCtrl,
                  label: 'Password',
                  prefixIcon: Icons.lock_outline_rounded,
                  obscureText: obscurePass,
                  enabled: !isLoading,
                  textInputAction: TextInputAction.done,
                  onSubmitted: (_) => onSubmit?.call(),
                  suffixIcon: IconButton(
                    icon: Icon(
                      obscurePass
                          ? Icons.visibility_outlined
                          : Icons.visibility_off_outlined,
                      size: 20,
                      color: NJColors.outline,
                    ),
                    onPressed: onTogglePass,
                    splashRadius: 18,
                  ),
                ),

                const SizedBox(height: NJSpacing.md),

                // ── Keep signed in ────────────────────────────
                Row(
                  children: [
                    SizedBox(
                      width: 20,
                      height: 20,
                      child: Checkbox(
                        value: keepSigned,
                        onChanged: isLoading ? null : onToggleKeep,
                        shape: RoundedRectangleBorder(
                          borderRadius: BorderRadius.circular(NJRadius.xs),
                        ),
                        side: const BorderSide(
                          color: NJColors.outline,
                          width: 1.5,
                        ),
                        activeColor: NJColors.primary,
                        materialTapTargetSize: MaterialTapTargetSize.shrinkWrap,
                      ),
                    ),
                    const SizedBox(width: NJSpacing.sm),
                    GestureDetector(
                      onTap: isLoading
                          ? null
                          : () => onToggleKeep(!keepSigned),
                      child: Text(
                        'Keep me signed in on this device',
                        style: NJText.bodyMd(),
                      ),
                    ),
                  ],
                ),

                const SizedBox(height: NJSpacing.lg),

                // ── Sign In button ────────────────────────────
                SizedBox(
                  width: double.infinity,
                  height: 52,
                  child: FilledButton(
                    onPressed: onSubmit,
                    style: FilledButton.styleFrom(
                      backgroundColor: NJColors.primary,
                      foregroundColor: NJColors.onPrimary,
                      shape: const StadiumBorder(),
                      elevation: 0,
                      disabledBackgroundColor:
                          NJColors.primary.withOpacity(0.6),
                    ),
                    child: isLoading
                        ? const SizedBox(
                            width: 20,
                            height: 20,
                            child: CircularProgressIndicator(
                              strokeWidth: 2,
                              color: NJColors.onPrimary,
                            ),
                          )
                        : Text(
                            'Sign In',
                            style: NJText.labelLg(color: NJColors.onPrimary),
                          ),
                  ),
                ),

                const SizedBox(height: NJSpacing.xl),

                // ── OR divider ────────────────────────────────
                Row(
                  children: [
                    const Expanded(
                      child: Divider(color: NJColors.outlineVariant),
                    ),
                    Padding(
                      padding: const EdgeInsets.symmetric(
                        horizontal: NJSpacing.md,
                      ),
                      child: Text('or', style: NJText.bodyMd()),
                    ),
                    const Expanded(
                      child: Divider(color: NJColors.outlineVariant),
                    ),
                  ],
                ),

                const SizedBox(height: NJSpacing.lg),

                // ── Footer links ──────────────────────────────
                Center(
                  child: Column(
                    children: [
                      TextButton(
                        onPressed: () {
                          // TODO: open browser to ERP password reset page
                          // url_launcher: launchUrl(Uri.parse('${ErpApi.base}/update-password'))
                        },
                        child: Text(
                          'Forgot password?',
                          style: NJText.labelLg(color: NJColors.primary),
                        ),
                      ),
                      const SizedBox(height: NJSpacing.xs),
                      RichText(
                        text: TextSpan(
                          style: NJText.bodyMd(),
                          children: [
                            const TextSpan(text: "Can't find your device?  "),
                            WidgetSpan(
                              alignment: PlaceholderAlignment.middle,
                              child: TextButton(
                                onPressed: () {},
                                style: TextButton.styleFrom(
                                  padding: EdgeInsets.zero,
                                  minimumSize: Size.zero,
                                  tapTargetSize:
                                      MaterialTapTargetSize.shrinkWrap,
                                ),
                                child: Text(
                                  'Contact Support',
                                  style: NJText.labelLg(
                                      color: NJColors.primary),
                                ),
                              ),
                            ),
                          ],
                        ),
                      ),
                    ],
                  ),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}

// ═══════════════════════════════════════════════════════════════════
// SUB-WIDGETS
// ═══════════════════════════════════════════════════════════════════

// ── Segmented toggle (Internal Login | Customer Verification) ──────
class _SegmentedToggle extends StatelessWidget {
  final bool isInternal;
  final ValueChanged<bool> onToggle;

  const _SegmentedToggle({
    required this.isInternal,
    required this.onToggle,
  });

  @override
  Widget build(BuildContext context) {
    return Container(
      height: 44,
      padding: const EdgeInsets.all(4),
      decoration: BoxDecoration(
        color: NJColors.surfaceContainerHigh,
        borderRadius: BorderRadius.circular(NJRadius.pill),
      ),
      child: Row(
        children: [
          _Tab(
            label: 'Internal Login',
            active: isInternal,
            onTap: () => onToggle(true),
          ),
          _Tab(
            label: 'Customer Verification',
            active: !isInternal,
            onTap: () => onToggle(false),
          ),
        ],
      ),
    );
  }
}

class _Tab extends StatelessWidget {
  final String label;
  final bool active;
  final VoidCallback onTap;

  const _Tab({
    required this.label,
    required this.active,
    required this.onTap,
  });

  @override
  Widget build(BuildContext context) {
    return Expanded(
      child: GestureDetector(
        onTap: onTap,
        child: AnimatedContainer(
          duration: const Duration(milliseconds: 180),
          curve: Curves.easeInOut,
          decoration: BoxDecoration(
            color: active ? NJColors.primary : Colors.transparent,
            borderRadius: BorderRadius.circular(NJRadius.pill),
            boxShadow: active
                ? [
                    BoxShadow(
                      color: NJColors.primary.withOpacity(0.25),
                      blurRadius: 6,
                      offset: const Offset(0, 2),
                    ),
                  ]
                : null,
          ),
          alignment: Alignment.center,
          child: Text(
            label,
            style: NJText.labelLg(
              color: active
                  ? NJColors.onPrimary
                  : NJColors.onSurfaceVariant,
            ),
          ),
        ),
      ),
    );
  }
}

// ── Outlined text field ───────────────────────────────────────────
class _NJField extends StatelessWidget {
  final TextEditingController controller;
  final String label;
  final IconData prefixIcon;
  final bool obscureText;
  final bool enabled;
  final Widget? suffixIcon;
  final TextInputAction? textInputAction;
  final TextInputType? keyboardType;
  final ValueChanged<String>? onSubmitted;

  const _NJField({
    required this.controller,
    required this.label,
    required this.prefixIcon,
    this.obscureText = false,
    this.enabled = true,
    this.suffixIcon,
    this.textInputAction,
    this.keyboardType,
    this.onSubmitted,
  });

  @override
  Widget build(BuildContext context) {
    return TextField(
      controller: controller,
      obscureText: obscureText,
      enabled: enabled,
      textInputAction: textInputAction,
      keyboardType: keyboardType,
      onSubmitted: onSubmitted,
      style: NJText.bodyLg(),
      decoration: InputDecoration(
        labelText: label,
        prefixIcon: Padding(
          padding: const EdgeInsets.symmetric(horizontal: NJSpacing.md),
          child: Icon(
            prefixIcon,
            size: 20,
            color: NJColors.outline,
          ),
        ),
        prefixIconConstraints:
            const BoxConstraints(minWidth: 48, minHeight: 48),
        suffixIcon: suffixIcon,
        // border and label style come from njTheme() InputDecorationTheme
      ),
    );
  }
}

// ── Error banner ──────────────────────────────────────────────────
class _ErrorBanner extends StatelessWidget {
  final String message;
  final VoidCallback onDismiss;

  const _ErrorBanner({required this.message, required this.onDismiss});

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(
        horizontal: NJSpacing.md,
        vertical: NJSpacing.sm,
      ),
      decoration: BoxDecoration(
        color: NJColors.errorContainer,
        borderRadius: BorderRadius.circular(NJRadius.sm),
        border: Border.all(
          color: NJColors.error.withOpacity(0.3),
        ),
      ),
      child: Row(
        children: [
          const Icon(Icons.error_outline_rounded,
              size: 18, color: NJColors.onErrorContainer),
          const SizedBox(width: NJSpacing.sm),
          Expanded(
            child: Text(
              message,
              style: NJText.bodySm(color: NJColors.onErrorContainer),
            ),
          ),
          GestureDetector(
            onTap: onDismiss,
            child: const Icon(Icons.close_rounded,
                size: 16, color: NJColors.onErrorContainer),
          ),
        ],
      ),
    );
  }
}

// ── Update warning banner ─────────────────────────────────────────
class _UpdateBanner extends StatelessWidget {
  final double? serverVersion;

  const _UpdateBanner({this.serverVersion});

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(
        horizontal: NJSpacing.md,
        vertical: NJSpacing.sm,
      ),
      decoration: BoxDecoration(
        color: NJColors.warningContainer,
        borderRadius: BorderRadius.circular(NJRadius.sm),
        border: Border.all(
          color: NJColors.warning.withOpacity(0.3),
        ),
      ),
      child: Row(
        children: [
          const Icon(Icons.warning_amber_rounded,
              size: 18, color: NJColors.warning),
          const SizedBox(width: NJSpacing.sm),
          Expanded(
            child: Text(
              serverVersion != null
                  ? 'Update required: v${serverVersion!.toStringAsFixed(1)} available. Contact IT to update this machine.'
                  : 'A newer version is available. Please update soon.',
              style: NJText.bodySm(color: NJColors.warning),
            ),
          ),
        ],
      ),
    );
  }
}
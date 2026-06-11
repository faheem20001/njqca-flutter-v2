// ─────────────────────────────────────────────────────────────────
// nj_theme.dart  —  NewJaisa Design System
// Font: Hanken Grotesk  |  Base unit: 8px
// ─────────────────────────────────────────────────────────────────

import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';

// ── Colours ───────────────────────────────────────────────────────
class NJColors {
  NJColors._();

  static const primary             = Color(0xFF005E95);
  static const onPrimary           = Color(0xFFFFFFFF);
  static const primaryContainer    = Color(0xFF1377B9);
  static const onPrimaryContainer  = Color(0xFFF6F8FF);
  static const primaryFixed        = Color(0xFFCFE5FF);

  static const secondary           = Color(0xFF496172);
  static const onSecondary         = Color(0xFFFFFFFF);
  static const secondaryContainer  = Color(0xFFCCE6FA);

  static const surface                  = Color(0xFFF7F9FB);
  static const surfaceContainerLowest   = Color(0xFFFFFFFF);
  static const surfaceContainerLow      = Color(0xFFF2F4F6);
  static const surfaceContainer         = Color(0xFFECEEF0);
  static const surfaceContainerHigh     = Color(0xFFE6E8EA);
  static const surfaceContainerHighest  = Color(0xFFE0E3E5);
  static const inverseSurface           = Color(0xFF2D3133);
  static const inverseOnSurface         = Color(0xFFEFF1F3);

  static const onSurface        = Color(0xFF191C1E);
  static const onSurfaceVariant = Color(0xFF404750);
  static const outline          = Color(0xFF707881);
  static const outlineVariant   = Color(0xFFC0C7D2);

  static const error            = Color(0xFFBA1A1A);
  static const onError          = Color(0xFFFFFFFF);
  static const errorContainer   = Color(0xFFFFDAD6);
  static const onErrorContainer = Color(0xFF93000A);

  static const success          = Color(0xFF1B6B3A);
  static const onSuccess        = Color(0xFFFFFFFF);
  static const successContainer = Color(0xFFB7F0CC);

  static const warning          = Color(0xFF7C5800);
  static const warningContainer = Color(0xFFFFDEA0);
}

// ── Spacing ───────────────────────────────────────────────────────
class NJSpacing {
  NJSpacing._();
  static const double xs       = 4.0;
  static const double sm       = 8.0;
  static const double md       = 16.0;
  static const double lg       = 24.0;
  static const double xl       = 32.0;
  static const double xxl      = 48.0;
}

// ── Radius ────────────────────────────────────────────────────────
class NJRadius {
  NJRadius._();
  static const double xs   = 4.0;
  static const double sm   = 8.0;
  static const double md   = 12.0;
  static const double lg   = 16.0;
  static const double xl   = 24.0;
  static const double pill = 999.0;
}

// ── Text styles ───────────────────────────────────────────────────
class NJText {
  NJText._();

  static TextStyle displayLg({Color color = NJColors.onSurface}) =>
      GoogleFonts.hankenGrotesk(fontSize: 40, fontWeight: FontWeight.w700,
          height: 1.12, letterSpacing: -0.5, color: color);

  static TextStyle headlineLg({Color color = NJColors.onSurface}) =>
      GoogleFonts.hankenGrotesk(fontSize: 28, fontWeight: FontWeight.w600,
          height: 1.28, color: color);

  static TextStyle headlineMd({Color color = NJColors.onSurface}) =>
      GoogleFonts.hankenGrotesk(fontSize: 22, fontWeight: FontWeight.w600,
          height: 1.3, color: color);

  static TextStyle titleLg({Color color = NJColors.onSurface}) =>
      GoogleFonts.hankenGrotesk(fontSize: 18, fontWeight: FontWeight.w500,
          height: 1.4, color: color);

  static TextStyle titleMd({Color color = NJColors.onSurface}) =>
      GoogleFonts.hankenGrotesk(fontSize: 16, fontWeight: FontWeight.w600,
          height: 1.4, color: color);

  static TextStyle bodyLg({Color color = NJColors.onSurface}) =>
      GoogleFonts.hankenGrotesk(fontSize: 16, fontWeight: FontWeight.w400,
          height: 1.5, color: color);

  static TextStyle bodyMd({Color color = NJColors.onSurfaceVariant}) =>
      GoogleFonts.hankenGrotesk(fontSize: 14, fontWeight: FontWeight.w400,
          height: 1.43, letterSpacing: 0.1, color: color);

  static TextStyle bodySm({Color color = NJColors.onSurfaceVariant}) =>
      GoogleFonts.hankenGrotesk(fontSize: 12, fontWeight: FontWeight.w400,
          height: 1.4, color: color);

  static TextStyle labelLg({Color color = NJColors.onSurface}) =>
      GoogleFonts.hankenGrotesk(fontSize: 14, fontWeight: FontWeight.w600,
          height: 1.43, letterSpacing: 0.1, color: color);

  static TextStyle labelMd({Color color = NJColors.onSurface}) =>
      GoogleFonts.hankenGrotesk(fontSize: 12, fontWeight: FontWeight.w600,
          height: 1.4, color: color);

  static TextStyle labelSm({Color color = NJColors.onSurfaceVariant}) =>
      GoogleFonts.hankenGrotesk(fontSize: 11, fontWeight: FontWeight.w500,
          height: 1.45, letterSpacing: 0.4, color: color);

  static TextStyle mono({Color color = NJColors.onSurface, double size = 13}) =>
      TextStyle(fontFamily: 'monospace', fontSize: size, color: color, height: 1.4);
}

// ── Theme ─────────────────────────────────────────────────────────
ThemeData njTheme() {
  final base = ThemeData(
    useMaterial3: true,
    colorScheme: const ColorScheme(
      brightness:              Brightness.light,
      primary:                 NJColors.primary,
      onPrimary:               NJColors.onPrimary,
      primaryContainer:        NJColors.primaryContainer,
      onPrimaryContainer:      NJColors.onPrimaryContainer,
      secondary:               NJColors.secondary,
      onSecondary:             NJColors.onSecondary,
      secondaryContainer:      NJColors.secondaryContainer,
      onSecondaryContainer:    NJColors.onSurface,
      tertiary:                NJColors.secondary,
      onTertiary:              NJColors.onSecondary,
      tertiaryContainer:       NJColors.surfaceContainer,
      onTertiaryContainer:     NJColors.onSurface,
      error:                   NJColors.error,
      onError:                 NJColors.onError,
      errorContainer:          NJColors.errorContainer,
      onErrorContainer:        NJColors.onErrorContainer,
      surface:                 NJColors.surface,
      onSurface:               NJColors.onSurface,
      surfaceContainerHighest: NJColors.surfaceContainerHighest,
      onSurfaceVariant:        NJColors.onSurfaceVariant,
      outline:                 NJColors.outline,
      outlineVariant:          NJColors.outlineVariant,
      inverseSurface:          NJColors.inverseSurface,
      onInverseSurface:        NJColors.inverseOnSurface,
      inversePrimary:          NJColors.primaryFixed,
    ),
  );

  return base.copyWith(
    scaffoldBackgroundColor: NJColors.surface,
    textTheme: GoogleFonts.hankenGroteskTextTheme(base.textTheme),

    inputDecorationTheme: InputDecorationTheme(
      filled:                false,
      contentPadding:        const EdgeInsets.symmetric(vertical: 16, horizontal: 14),
      labelStyle:            NJText.bodyMd(),
      floatingLabelStyle:    NJText.bodyMd(color: NJColors.primary).copyWith(fontSize: 12),
      floatingLabelBehavior: FloatingLabelBehavior.auto,
      enabledBorder: OutlineInputBorder(
        borderRadius: BorderRadius.circular(NJRadius.sm),
        borderSide:   const BorderSide(color: NJColors.outline, width: 1.0),
      ),
      focusedBorder: OutlineInputBorder(
        borderRadius: BorderRadius.circular(NJRadius.sm),
        borderSide:   const BorderSide(color: NJColors.primary, width: 2.0),
      ),
      errorBorder: OutlineInputBorder(
        borderRadius: BorderRadius.circular(NJRadius.sm),
        borderSide:   const BorderSide(color: NJColors.error, width: 1.0),
      ),
      focusedErrorBorder: OutlineInputBorder(
        borderRadius: BorderRadius.circular(NJRadius.sm),
        borderSide:   const BorderSide(color: NJColors.error, width: 2.0),
      ),
    ),

    elevatedButtonTheme: ElevatedButtonThemeData(
      style: ElevatedButton.styleFrom(
        backgroundColor: NJColors.primary,
        foregroundColor: NJColors.onPrimary,
        minimumSize:     const Size(double.infinity, 52),
        shape:           const StadiumBorder(),
        elevation:       0,
        textStyle:       NJText.labelLg(color: NJColors.onPrimary),
      ),
    ),

    outlinedButtonTheme: OutlinedButtonThemeData(
      style: OutlinedButton.styleFrom(
        foregroundColor: NJColors.primary,
        minimumSize:     const Size(double.infinity, 52),
        shape:           const StadiumBorder(),
        side:            const BorderSide(color: NJColors.primary, width: 1.5),
        textStyle:       NJText.labelLg(color: NJColors.primary),
      ),
    ),

    textButtonTheme: TextButtonThemeData(
      style: TextButton.styleFrom(
        foregroundColor: NJColors.primary,
        textStyle:       NJText.labelLg(color: NJColors.primary),
      ),
    ),

    checkboxTheme: CheckboxThemeData(
      fillColor: WidgetStateProperty.resolveWith((s) =>
          s.contains(WidgetState.selected) ? NJColors.primary : Colors.transparent),
      checkColor: WidgetStateProperty.all(NJColors.onPrimary),
      side:  const BorderSide(color: NJColors.outline, width: 1.5),
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(4)),
    ),

    dividerTheme: const DividerThemeData(
      color: NJColors.outlineVariant,
      thickness: 1,
      space: 0,
    ),
  );
}

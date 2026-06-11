import 'package:flutter/material.dart';
import '../../core/nj_theme.dart';

class ResultsScreen extends StatelessWidget {
  const ResultsScreen({super.key});
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: NJColors.surface,
      body: Center(
        child: Text('Results Screen — share Stitch prototype to build',
            style: NJText.titleLg()),
      ),
    );
  }
}

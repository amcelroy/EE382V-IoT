
import 'package:flutter/cupertino.dart';
import 'package:flutter_nfc_reader/flutter_nfc_reader.dart';
import 'package:nfc_manager/nfc_manager.dart';

// ignore: camel_case_types
class EE382V_IoT_NFC extends StatefulWidget{
  @override
  _EE382V_IoT_NFC createState() => _EE382V_IoT_NFC();
}

// ignore: camel_case_types
class _EE382V_IoT_NFC extends State<EE382V_IoT_NFC> {

  @override
  void initState() async {
    super.initState();

    FlutterNfcReader.read().then((response) {
      print(response.content);
    });

  }

  @override
  Widget build(BuildContext context) {
    return Column(
      children: [

      ],
    );
  }
}
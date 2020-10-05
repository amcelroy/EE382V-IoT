
import 'dart:typed_data';

import 'package:flutter/cupertino.dart';
import 'package:nfc_manager/nfc_manager.dart';
import 'package:nfc_manager/platform_tags.dart';

// ignore: camel_case_types
class EE382V_IoT_NFC extends StatefulWidget{
  @override
  _EE382V_IoT_NFC createState() => _EE382V_IoT_NFC();
}

// ignore: camel_case_types
class _EE382V_IoT_NFC extends State<EE382V_IoT_NFC> {

  Future _nfcThread;

  bool _stopThread = false;

  final int flags = 58; //Hex 0x3A

  void acquireThread(NfcTag tag) {
    _nfcThread = new Future(() {
      NfcManager.instance.startSession(
        onDiscovered: (NfcTag tag) async {
          NfcV t = NfcV.from(tag);
          _stopThread = false;

          List<int> data = new List<int>();
          data.add(flags);
          data.add(32);
          data.addAll(tag.data['id']);
          data.add(0);

          while(_stopThread == false){
            try{
              Uint8List response = await t.transceive(data: Uint8List.fromList(data));

              //TODO: Do something with the data

              await Future.delayed(Duration(milliseconds: 75));
            }on Exception catch (e) {
              _stopThread = true;
            }
          }
        },
      );
    });
  }

  @override
  void initState() async {
    super.initState();

    bool isAvailable = await NfcManager.instance.isAvailable();

    if(isAvailable){

    }else{

    }
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      children: [

      ],
    );
  }
}
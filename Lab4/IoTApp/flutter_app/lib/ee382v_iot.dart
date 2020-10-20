
import 'dart:async';
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

  int _microsSeconds = 0;

  final int flags = 58; //Hex 0x3A

  bool _emulation = true;

  void acquireThread() async {
    bool isAvailable = await NfcManager.instance.isAvailable();
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
              Stopwatch sw = new Stopwatch();
              sw.start();
              Uint8List response = await t.transceive(data: Uint8List.fromList(data));
              sw.stop();

              setState(() {
                _microsSeconds = sw.elapsedMicroseconds;
              });

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
  void initState() {
    super.initState();

    if(_emulation){
      Timer.periodic(new Duration(milliseconds: 75), (Timer t) async {
        Stopwatch sw = new Stopwatch();
        sw.start();
        await Future.delayed(Duration(milliseconds: 75));
        sw.stop();

        setState(() {
          _microsSeconds = sw.elapsedMicroseconds;
        });
      });
    }else{
      acquireThread();
    }
  }

  @override
  Widget build(BuildContext context) {
    if(_emulation){
      return
            new Column(
              mainAxisSize: MainAxisSize.min,
            children: [
              new Text("***Emulation Mode***"),
              new Text("Acq. Time for 32 Bytes:"),
              new Text(_microsSeconds.toString()),
            ],
      );
    }else{
      return Column(
        children: [
          new Center(
              child: new Column(
                children: [
                  new Text("Acq. Time for 32 Bytes:"),
                  new Text(_microsSeconds.toString()),
                ],
              )
          )
        ],
      );
    }
  }
}
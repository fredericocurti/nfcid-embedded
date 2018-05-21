package app.nfcbois.ndefred;

import android.nfc.NdefMessage;
import android.nfc.NdefRecord;
import android.nfc.NfcAdapter;
import android.nfc.NfcEvent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

public class MainActivity extends AppCompatActivity implements NfcAdapter.CreateNdefMessageCallback, NfcAdapter.OnNdefPushCompleteCallback {
    NfcAdapter mNfcAdapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mNfcAdapter = NfcAdapter.getDefaultAdapter(this);
        if(mNfcAdapter != null) {
            // Register callback to set NDEF message
            mNfcAdapter.setNdefPushMessageCallback(this, this);

            // Register callback to listen for message-sent success
            mNfcAdapter.setOnNdefPushCompleteCallback(this, this);
        } else {
            Log.i("LinkDetails", "NFC is not available on this device");
        }
    }


    @Override
    public NdefMessage createNdefMessage(NfcEvent event) {
        NdefMessage msg = new NdefMessage(new NdefRecord[] {
                NdefRecord.createUri("meuid")
        });
        return msg;
    }

    @Override
    public void onNdefPushComplete(NfcEvent event) {

    }
}

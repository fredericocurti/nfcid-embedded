package app.nfcbois.ndefred;

import android.animation.ArgbEvaluator;
import android.animation.ObjectAnimator;
import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.nfc.NdefMessage;
import android.nfc.NdefRecord;
import android.nfc.NfcAdapter;
import android.nfc.NfcEvent;
import android.support.constraint.ConstraintLayout;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity implements NfcAdapter.CreateNdefMessageCallback, NfcAdapter.OnNdefPushCompleteCallback {
    NfcAdapter mNfcAdapter;
    Button mButton;
    EditText mEdit;
    SharedPreferences settings;
    ConstraintLayout screen;
    SharedPreferences.Editor editor;
    String id = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        settings = getSharedPreferences("UserInfo", 0);
        editor = settings.edit();

        id = settings.getString("id", "");
        mButton = (Button)findViewById(R.id.button);
        mEdit   = (EditText)findViewById(R.id.form);
        screen = (ConstraintLayout)findViewById(R.id.layout);
        screen.requestFocus();

        mEdit.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                if (actionId == EditorInfo.IME_ACTION_DONE) {
                    mButton.performClick();
                }
                return false;
            }
        });

        // puts retrieved text;
        mEdit.setText(id);

        mButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.v("EditText", mEdit.getText().toString());
                id = mEdit.getText().toString();
                editor.putString("id", id);
                editor.apply();
            }
        });

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
                NdefRecord.createUri(id)
        });
        return msg;
    }

    @Override
    public void onNdefPushComplete(NfcEvent event) {
        Log.d("HEY", "SUCCESSS");
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(MainActivity.this, "Bem vindo!",
                        Toast.LENGTH_LONG).show();
            }
        });

    }
}

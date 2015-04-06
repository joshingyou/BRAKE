package com.bluecreation.melodysmart.testapp;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.bluecreation.melodysmart.ImageManager;
import com.bluecreation.melodysmart.ImageManagerListener;
import com.bluecreation.melodysmart.MelodySmartDevice;
import com.bluecreation.melodysmart.OtauListener;
import com.bluecreation.melodysmart.OtauRelease;
import com.bluecreation.melodysmart.R;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by genis on 17/10/2014.
 */
public class OtauTestActivity extends Activity implements ImageManagerListener, OtauListener {

    private static String TAG = OtauTestActivity.class.getSimpleName();
    private MelodySmartDevice device;
    private ImageManager imageManager;

    /* UI */
    private Button startButton;
    private ProgressBar progressBar;
    private TextView progressLabel;

    /* Array with all the available Melody Smart images */
    private ArrayList<String> listData = new ArrayList<>();
    private byte[] fwData;


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.ota_activity);
        /* Get the instance of the Library */
        device = MelodySmartDevice.getInstance();

        /* Get UI elements */
        startButton = (Button) findViewById(R.id.start_button);
        progressBar = (ProgressBar)  findViewById(R.id.progress_bar);
        progressLabel = (TextView)  findViewById(R.id.progress_label);

        /* Listen to changers on the OTAU protocol */
        device.registerListener(this);


        /* Start downloading the list of available Melody Smart images */
        imageManager = new ImageManager();
        imageManager.registerListener(this);
        imageManager.downloadList();



        /* Get the remote device Melody Smart version if it was already retrieved to the device */
        String version = device.getDeviceVersion();
        if (version != null) {
            ((TextView) findViewById(R.id.device_version_text)).setText(version);
        } else {
            ((TextView) findViewById(R.id.device_version_text)).setText("N/A");
        }

        /* Disable the OTA start button until device is ready for OTA */
        startButton.setEnabled(false);
        startButton.setText("Start OTAU");
        startButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startButton.setEnabled(false);
                device.startOtau(fwData);
            }
        });

    }

    @Override
    public void onDestroy() {
        imageManager.unregisterListener(this);
        device.unregisterListener(this);
        super.onDestroy();
    }

    @Override
    public void onListDownloaded(final List<OtauRelease> list) {


        OtauRelease n = new OtauRelease();
        n.setFileName("2014-10-21-melody_smart_2.7.0_alpha.img");
        n.setReleaseDate("2014-10-21");
        n.setVersion("2.7.0 alpha");
        list.add(n);

        for (OtauRelease release : list) {
            listData.add(release.toItemString());
        }
        /* Show list and let user select one */
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                AlertDialog.Builder builder = new AlertDialog.Builder(OtauTestActivity.this);
                builder.setTitle("Select image");
                builder.setItems(listData.toArray(new String[listData.size()]), new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        Log.d(TAG, String.format("Selected image from the list (%d/%d)", i, list.size()));
                        ((TextView) findViewById(R.id.online_version_text)).setText(list.get(i).getVersion());
                        imageManager.prepareImage(list.get(i), device.getBtAddress(), device.getXtalTrim());
                    }
                });
                builder.show();
            }
        });
    }

    @Override
    public void onImageDownloaded(final byte[] fwData) {
        this.fwData = fwData;
        startButton.setEnabled(true);
    }


    @Override
    public void onOtauProgressChanged(final int percentage) {
       // if(success) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    progressBar.setProgress(percentage);
                    progressLabel.setText(String.format("%d%%", percentage));
                }
            });
        //}
    }

    @Override
    public void onOtauFinished() {
        Log.d(TAG, "Preparing for reset");

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                final Button button = (Button) findViewById(R.id.start_button);
                button.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        OtauTestActivity.this.finish();
                    }
                });

                button.setText("Finish");
                progressLabel.setText("Completed");
                button.setEnabled(true);
            }
        });
    }
}

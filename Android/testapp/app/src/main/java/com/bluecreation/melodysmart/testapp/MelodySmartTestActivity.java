package com.bluecreation.melodysmart.testapp;

import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.location.Location;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.AutoCompleteTextView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.bluecreation.melodysmart.BatteryService;
import com.bluecreation.melodysmart.DataService;
import com.bluecreation.melodysmart.DeviceInfoService;
import com.bluecreation.melodysmart.MelodySmartDevice;
import com.bluecreation.melodysmart.MelodySmartListener;
import com.bluecreation.melodysmart.R;
import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.api.GoogleApiClient;
import com.google.android.gms.location.LocationListener;
import com.google.android.gms.location.LocationRequest;
import com.google.android.gms.location.LocationServices;
import com.google.android.gms.maps.CameraUpdate;
import com.google.android.gms.maps.CameraUpdateFactory;
import com.google.android.gms.maps.GoogleMap;
import com.google.android.gms.maps.SupportMapFragment;
import com.google.android.gms.maps.model.LatLng;
import com.google.android.gms.maps.model.LatLngBounds;
import com.google.android.gms.maps.model.Marker;
import com.google.android.gms.maps.model.MarkerOptions;
import com.google.android.gms.maps.model.PolylineOptions;

import org.json.JSONObject;

import java.io.IOException;
import java.net.URL;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * Created by genis on 15/10/2014.
 */
public class MelodySmartTestActivity extends FragmentActivity implements MelodySmartListener,GoogleApiClient.ConnectionCallbacks, GoogleApiClient.OnConnectionFailedListener, AdapterView.OnItemClickListener,
        LocationListener {

    private static final int OTAU_ACTIVITY_CODE = 1;
    private MelodySmartDevice device;
    private EditText dataToSend;
    private EditText dataReceived;
    private Button otaButton;
    private Button batteryButton;
    private AlertDialog connectingDialog;
    private Button sendBtn;
    private TextView directionLabel;

    private int nextInfoType = 0;

    protected static final String TAG = "brake";

    private Context mContext;

    private static final String DIRECTIONS_API_BASE = "https://maps.googleapis.com/maps/api/directions";
    private static final String PLACE_DETAILS_API_BASE = "https://maps.googleapis.com/maps/api/place/details";
    private static final String OUT_JSON = "/json";
    private static final String API_KEY = "AIzaSyDR7zE3-nILunUuaNd__w5ECgbETd_c33I";
    private static final String BROWSER_KEY = "AIzaSyCFjkcwHDGuWKIPEyGHl0cTeyQA7t6cOIE";

    //private ListView lvDir;
    private PlacesAutoCompleteAdapter PACA;
    private int dirIdx = 0;
    private ArrayList directionList;
    private DirectionInfo directionInfo;

    /**
     * Provides the entry point to Google Play services.
     */
    protected GoogleApiClient mGoogleApiClient;

    /**
     * Represents a geographical location.
     */
    private Location mLastLocation;
    private LocationRequest mLocationRequest;

    private PolylineOptions polyLineOptions = null;
    private GoogleMap mMap; // Might be null if Google Play services APK is not available.
    private Marker mMarker = null;

    private String destination;
    private URL placeQuery;

    private BatteryService.Listener batteryServiceListener = new BatteryService.Listener() {

        @Override
        public void onBatteryLevelChanged(final int level) {
            Log.d(TAG, "Got battery level " + level);
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    ((TextView) findViewById(R.id.batteryView)).setText(String.format("Battery : %d", level));
                }
            });
        }

        @Override
        public void onConnected(boolean found) {
            Log.d(TAG, "Connected to battery level service");
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    batteryButton.setEnabled(true);
                }
            });
            device.getBatteryService().enableNotifications(true);
        }
    };

    private DataService.Listener dataServiceListener = new DataService.Listener() {

        @Override
        public void onConnected(final boolean found) {
            if(found) {
                device.getDataService().enableNotifications(true);
            }
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    connectingDialog.cancel();
                    if (!found) {
                        Toast.makeText(MelodySmartTestActivity.this, "MelodySmart service not found on the remote device.", Toast.LENGTH_LONG).show();
                    }
                    dataToSend.setEnabled(found);
                    otaButton.setEnabled(device.isOtauAvailable());
                }
            });
        }

        @Override
        public void onReceived(final String data) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Log.d("data", "RECIEVED:" + data);
                    dataReceived.setText(data);
                }
            });
        }
    };


    private DeviceInfoService.Listener deviceInfoListener = new DeviceInfoService.Listener() {
        @Override
        public void onInfoRead(final DeviceInfoService.INFO_TYPE type, final String value) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    switch(type) {
                        case MANUFACTURER_NAME:
                            ((TextView)findViewById(R.id.manufacturerNameView)).setText("Manufacturer name: " + value);
                            break;
                        case MODEL_NUMBER:
                            ((TextView)findViewById(R.id.modelNumberView)).setText("Model number: " + value);
                            break;
                        case SERIAL_NUMBER:
                            ((TextView)findViewById(R.id.serialNumberView)).setText("Serial number: " + value);
                            break;
                        case HARDWARE_REV:
                            ((TextView)findViewById(R.id.hwRevView)).setText("HW rev: " + value);
                            break;
                        case FIRMWARE_REV:
                            ((TextView)findViewById(R.id.fwRevView)).setText("FW rev " + value);
                            break;
                        case SOFTWARE_REV:
                            ((TextView)findViewById(R.id.swRevView)).setText("SW rev: " + value);
                            break;
                        case SYSTEM_ID:
                            ((TextView)findViewById(R.id.sysIdView)).setText("System id: " + value);
                            break;
                        case PNP_ID:
                            ((TextView)findViewById(R.id.pnpIdView)).setText("PNP id: " + value);
                            break;
                    }
                }
            });
            readNextInfo();
        }

        @Override
        public void onConnected(boolean found) {
            /* Start reading all the characteristics */
            readNextInfo();
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.test_activity);
        Log.d(TAG, "Starting");

        sendBtn = (Button) findViewById(R.id.sendBtn);
        directionLabel = (TextView) findViewById(R.id.directionLabel);

        setUpMapIfNeeded();
        buildGoogleApiClient();
        AutoCompleteTextView autoCompView = (AutoCompleteTextView) findViewById(R.id.to);
        PACA = new PlacesAutoCompleteAdapter(this, R.layout.list_item);
        autoCompView.setAdapter(PACA);
        autoCompView.setOnItemClickListener(this);
        //lvDir = (ListView)findViewById(R.id.directionsList);

        mContext = (Context) getApplicationContext();

        // Create the LocationRequest object
        mLocationRequest = LocationRequest.create()
                .setPriority(LocationRequest.PRIORITY_HIGH_ACCURACY)
                .setInterval(10 * 1000)        // 10 seconds, in milliseconds
                .setFastestInterval(1 * 1000); // 1 second, in milliseconds

        /* Get the instance of the Melody Smart Android library and initialize it */
        device = MelodySmartDevice.getInstance();
        device.registerListener(this);
        device.getDataService().registerListener(dataServiceListener);
        device.getBatteryService().registerListener(batteryServiceListener);
        device.getDeviceInfoService().registerListener(deviceInfoListener);

        String deviceAddress = getIntent().getStringExtra("deviceAddress");
        String deviceName = getIntent().getStringExtra("deviceAddress");

        connectingDialog = new AlertDialog.Builder(this)
                .setMessage(String.format("Connecting to:\n%s\n(%s)...", deviceName, deviceAddress))
                .setTitle(R.string.app_name)
                .create();
        connectingDialog.show();

        try {
            device.connect(deviceAddress);
        } catch(Exception e) {
            Toast.makeText(this,e.getMessage(), Toast.LENGTH_LONG).show();
        }

        dataReceived = (EditText) findViewById(R.id.etReceivedData);
        dataToSend = (EditText) findViewById(R.id.etDataToSend);
        dataToSend.setOnEditorActionListener(new TextView.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView textView, int i, KeyEvent keyEvent) {
                device.getDataService().send(textView.getText().toString());
                return true;
            }
        });

        dataToSend.setEnabled(false);
        dataReceived.setEnabled(false);

        otaButton = (Button) findViewById(R.id.otaButton);
        otaButton.setEnabled(false);
        otaButton.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent intent = new Intent(getApplicationContext(), OtauTestActivity.class);
                startActivityForResult(intent, OTAU_ACTIVITY_CODE);
            }
        });

        batteryButton = (Button) findViewById(R.id.batteryButton);
        batteryButton.setEnabled(false);
        batteryButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                device.getBatteryService().read();
            }
        });

    }

    @Override
    protected void onActivityResult (int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
            case OTAU_ACTIVITY_CODE:
                finish();
                break;
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        mGoogleApiClient.connect();
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (mGoogleApiClient.isConnected()) {
            LocationServices.FusedLocationApi.removeLocationUpdates(mGoogleApiClient, this);
            mGoogleApiClient.disconnect();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        setUpMapIfNeeded();
    }

    protected synchronized void buildGoogleApiClient() {
        mGoogleApiClient = new GoogleApiClient.Builder(this)
                .addConnectionCallbacks(this)
                .addOnConnectionFailedListener(this)
                .addApi(LocationServices.API)
                .build();
    }


    @Override
    protected void onDestroy() {
        device.getDataService().unregisterListener(dataServiceListener);
        device.getBatteryService().unregisterListener(batteryServiceListener);
        device.getDeviceInfoService().unregisterListener(deviceInfoListener);
        device.unregisterListener(this);
        device.disconnect();
        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        //getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public void onDeviceConnected() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                connectingDialog.setMessage("Discovering MelodySmart service...");
            }
        });
    }

    @Override
    public void onDeviceDisconnected() {

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                dataToSend.setEnabled(false);
                Toast.makeText(MelodySmartTestActivity.this, "Disconnected from the device.", Toast.LENGTH_LONG).show();
            }
        });
    }

    @Override
    public void onOtauAvailable() {
        runOnUiThread(new Runnable(){
            @Override
            public void run() {
                otaButton.setEnabled(true);
            }
        });
    }

    private void readNextInfo() {
        DeviceInfoService.INFO_TYPE[] types = DeviceInfoService.INFO_TYPE.values();
        if (nextInfoType < types.length) {
            device.getDeviceInfoService().read(types[nextInfoType]);
            nextInfoType++;
        }
    }

    public void sendToGlove (View view) {
        Log.d("bluetooth", "SEND TO GLOVE");
        if (directionInfo != null) {
            List<NavigationPoint> navPoints = directionInfo.navigationPoints;
            String maneuver = navPoints.get(dirIdx).maneuver;
            int signal = 2;
            switch (maneuver) {
                case "go-strght":
                    signal = 0;
                    break;
                case "turn-left":
                    signal = 1;
                    break;
                case "turn-right":
                    signal = 2;
                    break;
            }
            if (mMarker != null) {
                mMarker.remove();
            }
            mMarker = mMap.addMarker(new MarkerOptions()
                    .position(new LatLng(navPoints.get(dirIdx).lat, navPoints.get(dirIdx).lon))
                    .title(maneuver));
            Log.d("bluetooth", "sending:" + signal);
            device.getDataService().send(Integer.toString(signal));
            directionLabel.setText((String) directionList.get(dirIdx));
            dirIdx++;
            if (dirIdx == navPoints.size()) {
                dirIdx = 0;
            }
        }

    }

    public void onItemClick(AdapterView<?> adapterView, View view, int position, long id) {
        String str = (String) adapterView.getItemAtPosition(position);
        destination = str;
        String placeID = PACA.getIdAtPosition(position);
        Log.d("placeid", placeID);
        Toast.makeText(this, str, Toast.LENGTH_SHORT).show();
        StringBuilder sb = new StringBuilder(PLACE_DETAILS_API_BASE + OUT_JSON);
        URL url = null;
        try {
            sb.append("?placeid=" + placeID);
            sb.append("&key=" + BROWSER_KEY);
            placeQuery = new URL(sb.toString());

        } catch (IOException e) {
            Log.e(TAG, "Error creating URL", e);
        }
    }

    public void displayDirections(View view) {
        InputMethodManager imm = (InputMethodManager) getSystemService(
                INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(getCurrentFocus().getWindowToken(), 0);
        if (mMap != null && polyLineOptions != null) {
            mMap.clear();
        }
        dirIdx = 0;
        directionLabel.setText("");
        // Perform action on click
        //URL url = createDirectionsUrl();
        ReadTask downloadTask = new ReadTask();
        downloadTask.execute(placeQuery);
    }

    private URL createDirectionsUrl(Double destLat, Double destLng) {
        StringBuilder sb = new StringBuilder(DIRECTIONS_API_BASE + OUT_JSON);
        double lat = mLastLocation.getLatitude();
        double lon = mLastLocation.getLongitude();
        URL url = null;
        AutoCompleteTextView textView = (AutoCompleteTextView) findViewById(R.id.to);

        try {
            //Log.d("end address", destination);
            sb.append("?origin=" + lat + "," + lon);
            sb.append("&destination=" + destLat + "," + destLng);
            sb.append("&mode=bicycling");
            sb.append("&key=" + BROWSER_KEY);

            url = new URL(sb.toString());
        } catch (IOException e) {
            Log.e(TAG, "Error creating URL", e);
        }
        return url;
    }

    @Override
    public void onLocationChanged(Location location) {
        handleNewLocation(location);
    }

    public void handleNewLocation(Location location) {
        Log.d(TAG, location.toString());

        double currentLatitude = location.getLatitude();
        double currentLongitude = location.getLongitude();
        LatLng latLng = new LatLng(currentLatitude, currentLongitude);

        /*MarkerOptions options = new MarkerOptions()
                .position(latLng)
                .title("I am here!");
        mMap.addMarker(options);
        mMap.moveCamera(CameraUpdateFactory.newLatLng(latLng));*/
    }

    private class ReadTask extends AsyncTask<URL, Void, String> {

        @Override
        protected String doInBackground(URL... url) {
            String data = "";
            String placeDetails = "";
            JSONObject jObject;
            try {
                HttpConnection http = new HttpConnection();
                placeDetails = http.readUrl(url[0]);
                jObject = new JSONObject(placeDetails);
                JSONObject latLng = (JSONObject) ((JSONObject) jObject.get("result")).get("geometry");
                Double lat = (Double) ((JSONObject) latLng.get("location")).get("lat");
                Double lng = (Double) ((JSONObject) latLng.get("location")).get("lng");

                Log.d("destination", "lat:" + lat + "," + "lng" + lng);

                URL dirURL = createDirectionsUrl(lat, lng);
                data = http.readUrl(dirURL);

            } catch (Exception e) {
                Log.d("Background Task", e.toString());
            }
            return data;
        }

        @Override
        protected void onPostExecute(String result) {
            super.onPostExecute(result);
            Log.d("data", result);
            new ParserTask().execute(result);
        }
    }

    private class ParserTask extends
            AsyncTask<String, Integer, DirectionInfo> {

        @Override
        protected DirectionInfo doInBackground(
                String... jsonData) {

            JSONObject jObject;
            List<List<HashMap<String, String>>> routes = null;
            DirectionInfo info = null;

            try {
                jObject = new JSONObject(jsonData[0]);
                PathJSONParser parser = new PathJSONParser();
                info = parser.parse(jObject);
            } catch (Exception e) {
                e.printStackTrace();
            }
            return info;
        }

        @Override
        protected void onPostExecute(DirectionInfo info) {
            ArrayList<LatLng> points = null;

            directionInfo = info;
            sendBtn.setEnabled(true);

            List<List<HashMap<String, String>>> routes = info.path;

            directionList = new ArrayList();
            DecimalFormat f = new DecimalFormat("##.000");
            for(NavigationPoint navPt : info.navigationPoints) {
                directionList.add(navPt.maneuver + ": " + f.format(navPt.lat) + "," + f.format(navPt.lon));
            }

            /*final ArrayAdapter adapter = new ArrayAdapter
                    (getApplicationContext(),android.R.layout.simple_list_item_1, directionList);
            lvDir.setAdapter(adapter);*/

            // traversing through routes
            for (int i = 0; i < routes.size(); i++) {
                points = new ArrayList<LatLng>();
                polyLineOptions = new PolylineOptions();
                List<HashMap<String, String>> path = routes.get(i);

                for (int j = 0; j < path.size(); j++) {
                    HashMap<String, String> point = path.get(j);

                    double lat = Double.parseDouble(point.get("lat"));
                    double lng = Double.parseDouble(point.get("lng"));
                    LatLng position = new LatLng(lat, lng);

                    points.add(position);
                }

                polyLineOptions.addAll(points);
                polyLineOptions.width(5);
                polyLineOptions.color(Color.BLUE);
            }

            mMap.addPolyline(polyLineOptions);
            LatLngBounds PATH = new LatLngBounds(
                    new LatLng(info.bounds.get(2), info.bounds.get(3)), new LatLng(info.bounds.get(0), info.bounds.get(1)));

            mMap.moveCamera(CameraUpdateFactory.newLatLngBounds(PATH, 15));
            startLocationListener();

        }
    }

    public void startLocationListener() {
        LocationServices.FusedLocationApi.requestLocationUpdates(mGoogleApiClient, mLocationRequest, this);
    }

    /**
     * Sets up the map if it is possible to do so (i.e., the Google Play services APK is correctly
     * installed) and the map has not already been instantiated.. This will ensure that we only ever
     * call {@link #setUpMap()} once when {@link #mMap} is not null.
     * <p/>
     * If it isn't installed {@link com.google.android.gms.maps.SupportMapFragment} (and
     * {@link com.google.android.gms.maps.MapView MapView}) will show a prompt for the user to
     * install/update the Google Play services APK on their device.
     * <p/>
     * A user can return to this FragmentActivity after following the prompt and correctly
     * installing/updating/enabling the Google Play services. Since the FragmentActivity may not
     * have been completely destroyed during this process (it is likely that it would only be
     * stopped or paused), {@link #onCreate(Bundle)} may not be called again so we should call this
     * method in {@link #onResume()} to guarantee that it will be called.
     */
    private void setUpMapIfNeeded() {
        // Do a null check to confirm that we have not already instantiated the map.
        if (mMap == null) {
            // Try to obtain the map from the SupportMapFragment.
            mMap = ((SupportMapFragment) getSupportFragmentManager().findFragmentById(R.id.map))
                    .getMap();
            // Check if we were successful in obtaining the map.
            if (mMap != null) {
                setUpMap();
            }
        }
    }

    /**
     * This is where we can add markers or lines, add listeners or move the camera. In this case, we
     * just add a marker near Africa.
     * <p/>
     * This should only be called once and when we are sure that {@link #mMap} is not null.
     */
    private void setUpMap() {
        //mMap.addMarker(new MarkerOptions().position(new LatLng(0, 0)).title("Marker"));
        mMap.setMyLocationEnabled(true);
    }

    /**
     * Runs when a GoogleApiClient object successfully connects.
     */
    @Override
    public void onConnected(Bundle connectionHint) {
        // Provides a simple way of getting a device's location and is well suited for
        // applications that do not require a fine-grained location and that do not need location
        // updates. Gets the best and most recent location currently available, which may be null
        // in rare cases when a location is not available.
        mLastLocation = LocationServices.FusedLocationApi.getLastLocation(mGoogleApiClient);
        if (mLastLocation != null) {
            //mMap.addMarker(new MarkerOptions().position(
            //        new LatLng(mLastLocation.getLatitude(), mLastLocation.getLongitude())).title("Marker"));
            LatLng latLng = new LatLng(mLastLocation.getLatitude(), mLastLocation.getLongitude());
            CameraUpdate cameraUpdate = CameraUpdateFactory.newLatLngZoom(latLng, 17);
            mMap.animateCamera(cameraUpdate);
            //mLatitudeText.setText(String.valueOf(mLastLocation.getLatitude()));
            //mLongitudeText.setText(String.valueOf(mLastLocation.getLongitude()));
        } else {
            Toast.makeText(this, "no location detected", Toast.LENGTH_LONG).show();
        }

        //LocationServices.FusedLocationApi.requestLocationUpdates(mGoogleApiClient, mLocationRequest, this);
    }

    @Override
    public void onConnectionFailed(ConnectionResult result) {
        // Refer to the javadoc for ConnectionResult to see what error codes might be returned in
        // onConnectionFailed.
        Log.i(TAG, "Connection failed: ConnectionResult.getErrorCode() = " + result.getErrorCode());
    }


    @Override
    public void onConnectionSuspended(int cause) {
        // The connection to Google Play services was lost for some reason. We call connect() to
        // attempt to re-establish the connection.
        Log.i(TAG, "Connection suspended");
        mGoogleApiClient.connect();
    }

}

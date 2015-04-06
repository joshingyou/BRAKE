package com.bluecreation.melodysmart.testapp;

import android.util.Log;

import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;

/**
 * Created by karenzeng on 3/15/15.
 */
public class HttpConnection {

    private static final String LOG_TAG = "Http";

    public String readUrl(URL url) throws IOException {

        StringBuilder jsonResults = new StringBuilder();
        InputStreamReader in = null;
        HttpURLConnection conn = null;
        try {

            conn = (HttpURLConnection) url.openConnection();
            in = new InputStreamReader(conn.getInputStream());

            // Load the results into a StringBuilder
            int read;
            char[] buff = new char[1024];
            while ((read = in.read(buff)) != -1) {
                jsonResults.append(buff, 0, read);
            }
            return jsonResults.toString();
        } catch (MalformedURLException e) {
            Log.e(LOG_TAG, "Error processing Places API URL", e);
            return null;
        } catch (IOException e) {
            Log.e(LOG_TAG, "Error connecting to Places API", e);
            return null;
        } finally {
            if (conn != null) {
                conn.disconnect();
            }
        }

    }
}

package com.bluecreation.melodysmart.testapp;

/**
 * Created by karenzeng on 3/15/15.
 */
public class NavigationPoint {

    double lat;
    double lon;
    String instructions;
    String maneuver = "";

    public NavigationPoint(double lat, double lon, String instructions, String maneuver) {
        this.lat = lat;
        this.lon = lon;
        this.instructions = instructions;
        this.maneuver = maneuver;
    }

}

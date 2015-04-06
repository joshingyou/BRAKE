package com.bluecreation.melodysmart.testapp;

import java.util.HashMap;
import java.util.List;

/**
 * Created by karenzeng on 3/15/15.
 */
public class DirectionInfo {

    List<Double> bounds;
    List<NavigationPoint> navigationPoints;
    List<List<HashMap<String, String>>> path;


    public DirectionInfo(List<Double> bounds, List<NavigationPoint> navigationPoints, List<List<HashMap<String, String>>> path) {
        this.bounds = bounds;
        this.navigationPoints = navigationPoints;
        this.path = path;

    }
}

package com.googlecode.gogodroid;

interface GogoServiceIface {
    void startGogoc();
    void stopGogoc();
    boolean statusGogoc();
    String statusConnection();
    String loadConf();
    void stateChanged();
}

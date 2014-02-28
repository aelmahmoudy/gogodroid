/**
 *  This file is part of GogoDroid.
 *  http://code.google.com/p/gogodroid
 *
 *  GogoDroid is open source software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  GogoDroid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GogoDroid.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  @author أحمد المحمودي (Ahmed El-Mahmoudy) <aelmahmoudy@sabily.org>
 */
package com.googlecode.gogodroid;

import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.app.Service;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

public final class GogoService extends Service {

  private GogoCtl ctl;
  private String lastStatus = "";
  private static final String LOGTAG = GogoService.class.getName();
  private static final int notifyID = 1;
  private Notification.Builder mNotifyBuilder = new Notification.Builder(this)
                                                        .setContentTitle("GogoDroid")
                                                        .setSmallIcon(R.drawable.icon);

  Thread monitorConnection = new Thread() {
    @Override
    public void run() {
      String oldStatus;
      long sleepPeriod;

      try {
        while(monitorConnection == Thread.currentThread()){
          sleepPeriod = 2000; // Default value = 2s
          if(lastStatus.startsWith("established")) {
            sleepPeriod = 60000; // 1m
          }
          Thread.currentThread().sleep(sleepPeriod);
          oldStatus = lastStatus;
          mBinder.statusConnection();
          if(!oldStatus.equals(lastStatus)) {
            // TODO: refreshUI callback
            if(lastStatus.startsWith("established")) {
              updateNotification("Connected: " + lastStatus.substring(12, lastStatus.length()), R.drawable.icon);
            }
            if(oldStatus.startsWith("established")) {
              updateNotification("Disconnected", R.drawable.offline);
            }
          }
          else {
            if(lastStatus.equals("not_available")) {
              // if gogoc is stopped, then no need to monitor connection status
              return;
            }
            if(lastStatus.equals("error")) {
              // TODO: restart gogoc ?
            }
          }
        }
      }
      catch (Exception e) {
        e.printStackTrace();
      }
    }
  };

  private final GogoServiceIface.Stub mBinder = new GogoServiceIface.Stub() {
    @Override
    public void startGogoc() throws RemoteException {
      ctl.startGogoc();
      if(!monitorConnection.isAlive()) {
        monitorConnection.start();
      }
    }

    @Override
    public void stopGogoc() throws RemoteException {
      ctl.stopGogoc();
    }

    @Override
    public boolean statusGogoc() throws RemoteException {
      return(ctl.statusGogoc());
    }

    @Override
    public String statusConnection() throws RemoteException {
      lastStatus = ctl.statusConnection();
      return(lastStatus);
    }

    @Override
    public String loadConf() throws RemoteException {
      return(ctl.loadConf());
    }

    @Override
    public void saveConf(String conf) throws RemoteException {
      ctl.saveConf(conf);
    }

    @Override
    public void stateChanged() throws RemoteException {
      ConnectivityManager connMgr = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
      NetworkInfo activeInfo = connMgr.getActiveNetworkInfo();
      SharedPreferences sharedPref = getSharedPreferences(Constants.PREFS_FILE, Context.MODE_PRIVATE);
      boolean autoConnect = sharedPref.getBoolean("auto_connect", false);

      if((activeInfo != null) && activeInfo.isConnected()) {
      // Connection available
        if(!statusGogoc()) { // gogoc not running
          // Start gogoc if auto_connect is set or gogoc connection was 
          // established or trying to connect:
          if(autoConnect || lastStatus.startsWith("established") || lastStatus.equals("connecting")) {
            startGogoc();
          }
        }
      }
      else if(statusGogoc()) { // Connection unavailable & gogoc is running
        // stop gogoc unless this is just a failover or there is a connection 
        // attempt. (just wait for the next broadcast after failover):
        if((activeInfo == null) || !activeInfo.isAvailable() || 
           (!activeInfo.isFailover() && 
            !activeInfo.isConnectedOrConnecting())) {
          stopGogoc();
          // TODO: finish() ?
        }
        else {
          // TODO: if isAvailable(): end after some user-defined timeout ?
        }
      }
    }
  };

  @Override
  public void onCreate() {
    super.onCreate();

    Log.d("GogoService", "created");
    ctl = new GogoCtl(this);
    ctl.init();
  }

  @Override
  public int onStartCommand(Intent intent, int flags, int startId) {
    super.onStartCommand(intent, flags, startId);

    try {
      mBinder.stateChanged();
    }
    catch(RemoteException e) {
      Log.e(Constants.LOG_TAG, "", e);
    }
    return(START_STICKY);
  }

  @Override
  public IBinder onBind(Intent intent) {
    //mIsBound = true;
    return mBinder;
  }

  public static Intent createIntent(Context context) {
    return new Intent(context, GogoService.class);
  }

  private void updateNotification(CharSequence text, int icon) {
    Context ctx = getApplicationContext();
    Intent notificationIntent = new Intent(ctx, GogoDroid.class);
    notificationIntent.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_FROM_BACKGROUND);
    PendingIntent contentIntent = PendingIntent.getActivity(ctx, notifyID, notificationIntent,
                                                            PendingIntent.FLAG_CANCEL_CURRENT);
    mNotifyBuilder.setContentIntent(contentIntent);
    mNotifyBuilder.setContentText(text);
    mNotifyBuilder.setSmallIcon(icon);

    NotificationManager mNotificationManager = 
      (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
    mNotificationManager.notify(notifyID, mNotifyBuilder.build());
  }

}

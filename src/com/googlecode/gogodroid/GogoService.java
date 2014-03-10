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
  private static final String LOG_TAG = GogoService.class.getName();
  private static final int notifyID = 1;
  private Notification.Builder mNotifyBuilder = new Notification.Builder(this)
                                                        .setContentTitle("GogoDroid")
                                                        .setSmallIcon(R.drawable.gogo6_icon);

  Thread monitorConnection;
  Intent refreshIntent;
  boolean userStopped = true;

  private final GogoServiceIface.Stub mBinder = new GogoServiceIface.Stub() {
    @Override
    public void startGogoc() throws RemoteException {
      saveConf();
      Log.d(LOG_TAG, "starting gogoc");
      ctl.startGogoc();
      userStopped = false;

      monitorConnection = new Thread() {
        @Override
        public void run() {
          String oldStatus;
          long sleepPeriod;

          try {
            Log.d(LOG_TAG, "monitorConnection thread started");
            while(monitorConnection == Thread.currentThread()){
              sleepPeriod = 2000; // Default value = 2s
              if(lastStatus.startsWith("established")) {
                sleepPeriod = 60000; // 1m
              }
              try {
                sleep(sleepPeriod);
              }
              catch (Exception e) {
                Log.e(LOG_TAG, "monitorConnection thread exception", e);
                e.printStackTrace();
              }
              oldStatus = lastStatus;
              statusConnection();
              if(!oldStatus.equals(lastStatus)) {
                Log.d(LOG_TAG, "monitorConnection status changed" + oldStatus + "->" + lastStatus);
                sendBroadcast(refreshIntent);
                if(lastStatus.startsWith("established")) {
                  updateNotification(getApplication().getString(R.string.notif_connected, lastStatus.substring(12, lastStatus.length())),
                                     R.drawable.gogo6_icon);
                }
                if(oldStatus.startsWith("established")) {
                  updateNotification(getApplication().getString(R.string.notif_disconnected),
                                     R.drawable.gogo6_offline);
                }
              }
              else {
                if(lastStatus.equals("not_available")) {
                  // if gogoc is stopped, then no need to monitor connection status
                  Log.d(LOG_TAG, "montionConnection thread break");
                  break;
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
      monitorConnection.start();
    }

    @Override
    public void stopGogoc(boolean userStop) throws RemoteException {
      Log.d(LOG_TAG, "stopping gogoc");
      userStopped = userStopped;
      ctl.stopGogoc();
      NotificationManager mNotificationManager = 
        (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
      mNotificationManager.cancel(notifyID);
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
          if(autoConnect || !userStopped) {
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
          stopGogoc(false);
        }
        else {
          // TODO: if isAvailable(): end after some user-defined timeout ?
        }
      }
    }

    private void saveConf() {
      StringBuilder conf = new StringBuilder();
      SharedPreferences sharedPref = getSharedPreferences(Constants.PREFS_FILE, Context.MODE_PRIVATE);

      conf.append("server=" + sharedPref.getString("server", Constants.DEFAULT_SERVER) + "\n");
      String auth_method = sharedPref.getString("auth_method", "anonymous");
      if(!auth_method.equals("anonymous")) {
        conf.append("auth_method=" + auth_method + "\n");
        conf.append("userid=" + sharedPref.getString("userid", "") + "\n");
        conf.append("passwd=" + sharedPref.getString("passwd", "") + "\n");
      }
      conf.append(sharedPref.getString("custom", "") + "\n");

      ctl.saveConf(conf.toString());
    }
  };

  @Override
  public void onCreate() {
    super.onCreate();

    Log.d(LOG_TAG, "service created");
    ctl = new GogoCtl(this);
    ctl.init();
    refreshIntent = new Intent(Constants.RefreshUIAction);
  }

  @Override
  public int onStartCommand(Intent intent, int flags, int startId) {
    super.onStartCommand(intent, flags, startId);

    Log.d(LOG_TAG, "onStartCommand");
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
    mNotifyBuilder.setAutoCancel(true);

    NotificationManager mNotificationManager = 
      (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
    mNotificationManager.notify(notifyID, mNotifyBuilder.getNotification());
  }

}

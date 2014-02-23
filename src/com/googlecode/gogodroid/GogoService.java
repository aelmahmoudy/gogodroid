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
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

public final class GogoService extends Service {

  private GogoCtl ctl;
  private static final String LOGTAG = GogoService.class.getName();

  private final GogoServiceIface.Stub mBinder = new GogoServiceIface.Stub() {
    @Override
    public void startGogoc() throws RemoteException {
      ctl.startGogoc();
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
      return(ctl.statusConnection());
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
      if(checkNetworkConnection()) {
        if(!statusGogoc()) {
          startGogoc();
        }
      }
      else if(statusGogoc()) {
          stopGogoc();
          // TODO: if isFailover then wait for another broadcast
      }
    }

    private boolean checkNetworkConnection() {
      ConnectivityManager connMgr = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
      try {
        NetworkInfo activeInfo = connMgr.getActiveNetworkInfo();
        return((activeInfo != null) && activeInfo.isConnected());
      }
      catch(Exception e) {
        Log.e(Constants.LOG_TAG, "connMgr=" + connMgr, e);
        return(false);
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

}

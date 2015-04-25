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
 *  @author Mariotaku Lee (mariotaku) <mariotaku.lee@gmail.com>
 */

package com.googlecode.gogodroid;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.Writer;
import java.io.InputStreamReader;
import java.io.InputStream;
import java.io.FileOutputStream;

import android.util.Log;

import android.widget.Toast;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Context;
import android.annotation.SuppressLint;
import android.os.Build;

import com.googlecode.gogodroid.R;

public class GogoCtl {
  private static final String EXEC_NAME = "libgogoc_exec.so";

	private String gogocExec;
	private String gogocConf;

  Context context;

  public GogoCtl(Context cxt) {
    context = cxt;
    gogocExec = getGogocExec(context);
    gogocConf = context.getApplicationInfo().dataDir + File.separator + "files/gogoc.conf";
  }

  public void init() {
		// load ipv6 and tun modules
    loadModules();
		//check whether busybox installed
    checkBusyBox();
  }

  @SuppressWarnings("static-access")
  public void startGogoc() {
    //check again whether device is supported, if not, show a error dialog.
    File tundev = new File (Constants.TUNDEV);
    File if_inet6 = new File (Constants.IF_INET6);
    if (!tundev.exists()){
      showDialog(R.string.tun_not_supported,R.string.tun_not_supported_details);
      return;
    }
    if (!if_inet6.exists()){
      showDialog(R.string.ipv6_not_supported,R.string.ipv6_not_supported_details);
      return;
    }

    Thread thread = new Thread() {
      @Override
      public void run() {
        Utils.runSuCommand(gogocExec + " -y -f " + gogocConf);
      }
    };
    thread.start();
    // sleep to give some time to statusGogoc for detecting process
    try{
      Thread.currentThread().sleep(2000);//sleep for 2000 ms
    }
    catch(Exception e){
      e.printStackTrace();
    }
  }
  
  
  @SuppressWarnings("static-access")
  public void stopGogoc() {
    
    Thread thread = new Thread() {
      @Override
      public void run() {
        Utils.runSuCommand("kill -9 " + getPid());
      }
    };
    thread.start();
    // sleep to give some time to statusGogoc to detect process
    try{
      Thread.currentThread().sleep(2000);//sleep for 2000 ms
    }
    catch(Exception e){
      e.printStackTrace();
    }
  }

  public boolean statusGogoc() {
    boolean run;
    String line;

    run=false;

    try {
      Process process = Runtime.getRuntime().exec("ps");
      process.waitFor();
      BufferedReader stdInput = new BufferedReader(new InputStreamReader(process.getInputStream()));
      while ( (line = stdInput.readLine()) != null ) {
        if ( line.contains(gogocExec) ) {
          run = true;
        }
      }
      Log.d(Constants.LOG_TAG, "statusGogoc() ='"+run+"'");
    }
    catch (Exception e) {
      e.printStackTrace();
    }
    return run;
  }

  public String statusConnection() {
    String linkstatus;
    linkstatus = "not_available";
    if (statusGogoc())  {
      linkstatus = "connecting";
      try {
        String line;
        BufferedReader bufferedreader = new BufferedReader(new FileReader(Constants.IF_INET6), 1024);
        while ((line = bufferedreader.readLine()) != null) {
          if (line.startsWith("fe80") || line.startsWith("0000")) {
            continue;
          }
          if (line.contains("tun")) {
            StringBuilder stringbuilder = new StringBuilder("");
            for (int i = 0; i < 8; i++) {
              stringbuilder.append(line.substring(i * 4, (i + 1) * 4));
              stringbuilder.append(i == 7 ? "" : ":");
            }
            String IP6Addr = stringbuilder.toString()
                                          .replaceAll(":(0000)+", ":")
                                          .replaceFirst("::+", "::");
            linkstatus = "established " + IP6Addr;
            break;
          }
        }
        bufferedreader.close();
      }
      catch (Exception e) {
        linkstatus = "error";
      }
    }
    Log.d(Constants.LOG_TAG, "statusConnection() status=" + linkstatus);
    return linkstatus;
  }

  public String loadConf() {
    String Config="";
    File gogoc_conf = new File (gogocConf);

    if( ! gogoc_conf.exists() ) {
      return Config;
    }

    try {
      BufferedReader in = new BufferedReader(new FileReader(gogocConf));
      String str;

      while ((str = in.readLine()) != null) {
        Config += str + "\n";
      }
      in.close();
    }
    catch (Exception ex) {
      Log.d(Constants.LOG_TAG, "Cannot read gogoc.conf");
      return Config;
    }

    return Config;
  }

  public void saveConf(String conf) {
    Writer output = null;
    try {
      output = new BufferedWriter(new FileWriter(gogocConf));
      output.write( conf );
      output.close();
    }
    catch (Exception e) {
      e.printStackTrace();
    }
  }

  /*         GUI methods         */
  public void showToast(int txt) {
    int text = txt;
    int duration = Toast.LENGTH_LONG;
    Toast toast = Toast.makeText(context, text, duration);
    toast.show();
  }

  public void showDialog(int title, int message) {
    new AlertDialog.Builder(context)
      .setTitle(title)
      .setMessage(message)
      .setNegativeButton(R.string.btn_ok, new DialogInterface.OnClickListener(){
      public void onClick(DialogInterface di, int i) {
      }
    })
    .show();
  }

  private String getPid() {
    String pid;
    String temp;
    pid = "";
    try{
      Process process = Runtime.getRuntime().exec("ps");
      process.waitFor();

      BufferedReader stdInput = new BufferedReader(new InputStreamReader(process.getInputStream()));
      while ( (temp = stdInput.readLine()) != null ) {
        if ( temp.contains(gogocExec) ) {
          String [] cmdArray = temp.split(" +");
          pid = cmdArray[1];
        }
      }

    }
    catch (Exception e) {
      e.printStackTrace();
    }
    return pid;
  }

  private void checkBusyBox() {
    String line;

    try {
      Process process = Runtime.getRuntime().exec("busybox");
      process.waitFor();
      BufferedReader stdInput = new BufferedReader(new InputStreamReader(process.getInputStream()));
      line = stdInput.readLine();
        if ( line.contains("BusyBox") ) {
          Log.d(Constants.LOG_TAG, "checkBusyBox() = installed");
        }
        else {
          Log.d(Constants.LOG_TAG, "checkBusyBox() = not_installed");
          showDialog(R.string.busybox_not_installed,R.string.busybox_not_installed_details);
        }
    }
    catch (Exception e) {
      e.printStackTrace();
    }
  }

  private void loadModules(){
    File tundev = new File (Constants.TUNDEV);
    File if_inet6 = new File (Constants.IF_INET6);

    try {
      //check if /proc/net/if_inet6 exists
      if ( ! tundev.exists()) {
        Log.d(Constants.LOG_TAG, "loadModule() cmd='modprobe tun'");
        Utils.runSuCommand("modprobe tun");
      }
      //check if /proc/net/if_inet6 exists
      if ( ! if_inet6.exists()){
        Log.d(Constants.LOG_TAG, "loadModule() cmd='modprobe ipv6");
        Utils.runSuCommand("modprobe ipv6");
      }
      Thread.sleep(2000L);
    }
    catch (Exception e) {
      e.printStackTrace();
    }
  }

  /**
   * Get full path to lib directory of app
   * 
   * @return dir as String
   */
  @SuppressLint("NewApi")
  private static String getGogocExec(Context context) {
      if (Build.VERSION.SDK_INT >= 9) {
          return context.getApplicationInfo().nativeLibraryDir + File.separator + EXEC_NAME;
      } else {
          return context.getApplicationInfo().dataDir + File.separator + "lib" + File.separator + EXEC_NAME;
      }
  }
}

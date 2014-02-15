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

import com.googlecode.gogodroid.R;

public class GogoCtl {

  Context context;
  public GogoCtl(Context cxt) {
    context = cxt;
  }

  public void init() {
		//install gogoc binary
    updateBinary();
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

    if( ! new File(Constants.GOGOC_BIN).exists() ) {
      updateBinary();
      return;
    }
    
    Thread thread = new Thread() {
      @Override
      public void run() {
        Utils.runSuCommand(Constants.GOGOC_BIN + " -y -f " + Constants.GOGOC_CONF);
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
        if ( line.contains(Constants.GOGOC_BIN) ) {
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

  public String loadConf() {
    String Config="";
    File gogoc_conf = new File (Constants.GOGOC_CONF);

    // create gogoc.conf
    if( ! gogoc_conf.exists() ) {
      showToast(R.string.conf_not_exists);
      for (int i=0; i< Constants.DEFAULT_CONF.length; i++){
        Config += Constants.DEFAULT_CONF[i] + "\n";
      }
      return Config;
    }

    try {
      BufferedReader in = new BufferedReader(new FileReader(Constants.GOGOC_CONF));
      String str;

      while ((str = in.readLine()) != null) {
        Config += str + "\n";
      }
      in.close();
    }
    catch (Exception ex) {
      showToast(R.string.cant_read_conf);
      return Config;
    }

    return Config;
  }

  public void saveConf(String conf) {
    Writer output = null;
      try {
        output = new BufferedWriter(new FileWriter(Constants.GOGOC_CONF));
        output.write( conf );
        output.close();
        Log.d(Constants.LOG_TAG, "saveConf() saved and closed");
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
    Log.d(Constants.LOG_TAG, "showToast() txt='"+txt+"'");
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
        //Log.d(Constants.LOG_TAG, "stopGogoc() temp='"+temp+"'");
        if ( temp.contains(Constants.GOGOC_BIN) ) {
          Log.d(Constants.LOG_TAG, "statusGogoc() temp='"+temp+"'");
          String [] cmdArray = temp.split(" +");
          pid = cmdArray[1];
        }
      }

    }
    catch (Exception e) {
      e.printStackTrace();
    }
    Log.d(Constants.LOG_TAG, "statusGogoc() pid='"+pid+"'");
    return pid;
  }

  private void updateBinary() {
    File gogoc_working_folder = new File(Constants.GOGOC_DIR);
    File gogoc_binary = new File(Constants.GOGOC_BIN);
    
    // create gogoc working directory
    if(!gogoc_working_folder.exists())
    {
      Log.d(Constants.LOG_TAG, "Creating "+Constants.GOGOC_DIR+" folder");
      gogoc_working_folder.mkdir();
    }
    
    // install gogoc binary
    if(!gogoc_binary.exists())
    {
      copyRaw(R.raw.gogoc, (Constants.GOGOC_BIN));
      showToast(R.string.binary_installed);
    }
    
    // change permission to executable
    Utils.runCommand("if [ ! -x " + Constants.GOGOC_BIN + " ];then chmod 755 " + Constants.GOGOC_BIN + ";fi");
  }
	
	private void copyRaw(int id,String path)
	{
		try {
			InputStream ins = context.getResources().openRawResource(id);
			int size = ins.available();

			// Read the entire resource into a local byte buffer.
			byte[] buffer = new byte[size];
			ins.read(buffer);
			ins.close();

			FileOutputStream fos = new FileOutputStream(path);
			fos.write(buffer);
			fos.close();
		}
		catch(Exception e){
			e.printStackTrace();
		}
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
}

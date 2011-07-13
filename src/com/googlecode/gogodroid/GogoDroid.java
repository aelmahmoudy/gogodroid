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
import java.io.IOException;
import java.io.Writer;
import java.io.InputStreamReader;
import java.io.InputStream;
import java.io.FileOutputStream;

import android.util.Log;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.googlecode.gogodroid.R;

public class GogoDroid extends Activity {
	
	private static final String LOG_TAG = "Gogoc";
	private static final String GOGOC_CONF= "/data/data/com.googlecode.gogodroid/files/gogoc.conf";
	private static final String GOGOC_DIR= "/data/data/com.googlecode.gogodroid/files/";
	private static final String GOGOC_BIN= "/data/data/com.googlecode.gogodroid/files/gogoc";
	private static final String DNS1 = "210.51.191.217";
	private static final String TUNDEV = "/dev/tun";
	private static final String IF_INET6 = "/proc/net/if_inet6";
	private static final String [] DEFAULT_CONF = {"server=anonymous.freenet6.net"};

	
	RadioButton StatusRunning;
	Button btnStart;
	Button btnStop;
	Button btnDNS;
	EditText gogocConfig;
	EditText currentIP;
	TextView conftxt;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        StatusRunning = (RadioButton) findViewById(R.id.Running);
        gogocConfig = (EditText) findViewById(R.id.GogocConf);
        currentIP = (EditText) findViewById(R.id.Address);
        conftxt = (TextView) findViewById(R.id.ConfText);

        
		btnStart = (Button) findViewById(R.id.ButtonStart);
		btnStart.setOnClickListener(new OnClickListener() {
		
			public void onClick(View v) {
				
				// load ipv6 and tun modules
				loadModules();
				
				//check again whether device is supported, if not, show a error dialog.
				File tundev = new File (TUNDEV);
				File if_inet6 = new File (IF_INET6);
				if (tundev.exists()&&if_inet6.exists()){
					// save conf in file
					saveConf();
					// check if gogoc running
					if ( statusGogoc()) {
						showToast(R.string.gogoc_already_running);
						}
					else {
						startGogoc();
						showToast(R.string.gogoc_started);
						Log.d(LOG_TAG, "onCreate() Gogoc started.");
						try {
							while(true){
								Thread.sleep(2000);
								if(statusConnection()=="established")
									break;
								}
						}
						catch (Exception e) {
							e.printStackTrace();
						}
						setResult(android.app.Activity.RESULT_OK);
						}
				}
				else if (!tundev.exists()){
					showDialog(R.string.tun_not_supported,R.string.tun_not_supported_details);
					}
				else if (!if_inet6.exists()){
					showDialog(R.string.ipv6_not_supported,R.string.ipv6_not_supported_details);
					}
				}
			});
		
		
		btnStop = (Button) findViewById(R.id.ButtonStop);
		btnStop.setOnClickListener(new OnClickListener() {

			public void onClick(View v) {
				if ( statusGogoc() ){
					stopGogoc();
					statusConnection();
					showToast(R.string.gogoc_stopped);
					setResult(android.app.Activity.RESULT_OK);
				}
				else {
					showToast(R.string.gogoc_not_running);
				}
			}

		});
		
		btnDNS = (Button) findViewById(R.id.ButtonDNS);
		btnDNS.setOnClickListener(new OnClickListener() {

			public void onClick(View v) {
				setDNS();
				showToast(R.string.dns_changed);
			}
			
		});
		
		//install gogoc binary
		updateBinary();
		
		//check whether busybox installed
		checkBusyBox();
		
		// load configuration in gogocConfig
		gogocConfig.setText( loadConf().toString() );
		
		// save configuration
		saveConf();
		
		// check gogodroid status
		statusConnection();
    }
    

    @SuppressWarnings("static-access")
	public void startGogoc() {
    	if( ! new File(GOGOC_BIN).exists() ) {
			updateBinary();
			return;
		}
    	
		Thread thread = new Thread() {
			@Override
			public void run() {
				Utils.runSuCommand(GOGOC_BIN + " -y -f " + GOGOC_CONF);
			}
		};
		thread.start();
		// sleep to give some time to statusGogoc for detecting process
		try{
		  Thread.currentThread().sleep(2000);//sleep for 2000 ms
		}
		catch(InterruptedException e){
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
		catch(InterruptedException e){
			e.printStackTrace();
		}
    }

    
    @SuppressWarnings("static-access")
	public void setDNS(){
    	
		Thread thread = new Thread() {
			@Override
			public void run() {
	    		Utils.runSuCommand("setprop net.dns1 " + DNS1 );
			}
		};
		thread.start();
		// sleep to give some time to statusGogoc to detect process
		try{
		  Thread.currentThread().sleep(1000);//sleep for 1000 ms
		}
		catch(InterruptedException e){
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
				if ( line.contains(GOGOC_BIN) ) {
					run = true;
				}
			}
			Log.d(LOG_TAG, "statusGogoc() ='"+run+"'");
	    }
	    catch (IOException e) {
			e.printStackTrace();
		}
	    catch (InterruptedException e) {
			e.printStackTrace();
		}
    	return run;
    }
    
    
    public void checkBusyBox() {
    	String line;

    	try {
			Process process = Runtime.getRuntime().exec("busybox");
			process.waitFor();
			BufferedReader stdInput = new BufferedReader(new InputStreamReader(process.getInputStream()));
			line = stdInput.readLine();
				if ( line.contains("BusyBox") ) {
					Log.d(LOG_TAG, "checkBusyBox() = installed");
				}
				else {
					Log.d(LOG_TAG, "checkBusyBox() = not_installed");
					showDialog(R.string.busybox_not_installed,R.string.busybox_not_installed_details);
				}
	    }
	    catch (IOException e) {
			e.printStackTrace();
		}
	    catch (InterruptedException e) {
			e.printStackTrace();
		}
    }
    
    
	public String loadConf() {
		String Config="";
		File gogoc_conf = new File (GOGOC_CONF);
		
		// create gogoc.conf
		if( ! gogoc_conf.exists() ) {
			showToast(R.string.conf_not_exists);
			for (int i=0; i< DEFAULT_CONF.length; i++){
				Config += DEFAULT_CONF[i] + "\n";
			}
			return Config;
		}
		
		
		try {
	        BufferedReader in = new BufferedReader(new FileReader(GOGOC_CONF));
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
	
	
	public String getPid() {
    	String pid;
    	String temp;
    	pid = "";
    	try{
    		Process process = Runtime.getRuntime().exec("ps");
			process.waitFor();
			
			BufferedReader stdInput = new BufferedReader(new InputStreamReader(process.getInputStream()));
			while ( (temp = stdInput.readLine()) != null ) {
				//Log.d(LOG_TAG, "stopGogoc() temp='"+temp+"'");
				if ( temp.contains(GOGOC_BIN) ) {
					Log.d(LOG_TAG, "statusGogoc() temp='"+temp+"'");
					String [] cmdArray = temp.split(" +");
					pid = cmdArray[1];
				}
			}
    		
    	}
    	catch (IOException e) {
			e.printStackTrace();
		}
	    catch (InterruptedException e) {
			e.printStackTrace();
		}
	    Log.d(LOG_TAG, "statusGogoc() pid='"+pid+"'");
		return pid;
	}
	

	public String statusConnection() {
		String linkstatus;
		linkstatus = "not_available";
		showIndicator("not_available");
		if (statusGogoc())  {
			linkstatus = "connecting";
			showIndicator("connecting");
			try {
				String line;
				BufferedReader bufferedreader = new BufferedReader(new FileReader(IF_INET6), 1024);
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
						currentIP.setText(stringbuilder.toString()
							.replaceAll(":(0000)+", ":")
							.replaceFirst("::+", "::"));
						linkstatus = "established";
						showIndicator("established");
						break;
					}
				}
				bufferedreader.close();
			} catch (Exception e) {
				linkstatus = "error";
				showIndicator("error");
			}
		}
		Log.d(LOG_TAG, "statusConnection() returns=" + linkstatus);
		return linkstatus;
	}
	
	
	public void loadModules(){
		File tundev = new File (TUNDEV);
		File if_inet6 = new File (IF_INET6);

		//check if /proc/net/if_inet6 exists
		if ( ! tundev.exists()){
			Log.d(LOG_TAG, "loadModule() cmd='modprobe tun'");
			Utils.runSuCommand("modprobe tun");
			}
		//check if /proc/net/if_inet6 exists
    	if ( ! if_inet6.exists()){
    		Log.d(LOG_TAG, "loadModule() cmd='modprobe ipv6");
    		Utils.runSuCommand("modprobe ipv6");
    		}
	}


	public void updateBinary() {
		File gogoc_working_folder = new File(GOGOC_DIR);
		File gogoc_binary = new File(GOGOC_BIN);
		
		// create gogoc working directory
		if(!gogoc_working_folder.exists())
		{
			Log.d(LOG_TAG, "Creating "+GOGOC_DIR+" folder");
			gogoc_working_folder.mkdir();
		}
		
		// install gogoc binary
		if(!gogoc_binary.exists())
		{
			copyRaw(R.raw.gogoc, (GOGOC_BIN));
			showToast(R.string.binary_installed);
		}
		
		// change permission to executable
		Utils.runCommand("if [ ! -x " + GOGOC_BIN + " ];then chmod 755 " + GOGOC_BIN + ";fi");
	}

	
	public void copyRaw(int id,String path)
	{
		try {
			InputStream ins = getResources().openRawResource(id);
			int size = ins.available();

			// Read the entire resource into a local byte buffer.
			byte[] buffer = new byte[size];
			ins.read(buffer);
			ins.close();

			FileOutputStream fos = new FileOutputStream(path);
			fos.write(buffer);
			fos.close();
		}
		catch (Exception e){}
	}
	
	
    public void setPermission(String file, String mode)
    {
        Utils.runCommand("chmod "+ mode + " " + file);
    }
	
	
	public void saveConf() {
		Writer output = null;
	    try {
	    	output = new BufferedWriter(new FileWriter(GOGOC_CONF));
	    	output.write( gogocConfig.getText().toString() );
	    	output.close();
	    	Log.d(LOG_TAG, "saveConf() saved and closed");
	    }
	    catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	
	public void showIndicator(String status) {
		if (status == "not_available"){
			currentIP.setText( R.string.not_available );
			StatusRunning.setPressed(false);
			StatusRunning.setChecked(false);
			gogocConfig.setFocusable(true);
		}
		if (status == "connecting"){
			currentIP.setText(R.string.gogoc_connecting);
			StatusRunning.setPressed(true);
			StatusRunning.setChecked(false);
			gogocConfig.setFocusable(false);
		}
		if (status == "established"){
			StatusRunning.setPressed(false);
			StatusRunning.setChecked(true);
			gogocConfig.setFocusable(false);
		}
		if (status == "error"){
			currentIP.setText(R.string.status_error);
			StatusRunning.setPressed(false);
			StatusRunning.setChecked(false);
			gogocConfig.setFocusable(true);
		}
	}
	
	
	public void showToast(int txt) {
		int text = txt;
		int duration = Toast.LENGTH_LONG;
		Toast toast = Toast.makeText(this, text, duration);
		toast.show();
		Log.d(LOG_TAG, "showToast() txt='"+txt+"'");
	}
	
	
	public void showDialog(int title, int message) {
			new AlertDialog.Builder(this)
				.setTitle(title)
				.setMessage(message)
				.setNegativeButton(R.string.btn_ok, new DialogInterface.OnClickListener(){
				public void onClick(DialogInterface di, int i) {
				}
			})
			.show();
	}
	
	
}
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
 *  along with TunnelDroid.  If not, see <http://www.gnu.org/licenses/>.
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
import java.io.OutputStream;

import android.util.Log;
import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.googlecode.gogodroid.R;

public class GogoDroid extends Activity {
	
	private static final String LOG_TAG = "Gogoc";
	private static final String GOGOC_DIR= "/sdcard/.gogoc/";
	private static final String GOGOC_CONF= "/sdcard/.gogoc/gogoc.conf";
	private static final String GOGOC_WORKING_DIR= "/data/data/com.googlecode.gogodroid/files/";
	private static final String GOGOC_BIN= "/data/data/com.googlecode.gogodroid/files/gogoc";
	private static final String DNS1 = "210.51.191.217";
	private static final String TUNDEV = "/dev/tun";
	private static final String IF_INET6 = "/proc/net/if_inet6";
	private static final String [] DEFAULT_CONF = {"server=anonymous.freenet6.net"};
	private Process process;

	
	CheckBox CheckboxRunning;
	Button btnStart;
	Button btnStop;
	Button btnDNS;
	EditText txtBox;
	TextView conftxt;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        CheckboxRunning = (CheckBox) findViewById(R.id.Running);
        txtBox = (EditText) findViewById(R.id.GogocConf);
        conftxt = (TextView) findViewById(R.id.ConfText);
        
		btnStart = (Button) findViewById(R.id.ButtonStart);
		btnStart.setOnClickListener(new OnClickListener() {
		
			public void onClick(View v) {
		    	String pid;
		    	String temp;
		    	pid = "";
		    	int i;
				try{
					process = Runtime.getRuntime().exec("ps");
					process.waitFor();
			
					BufferedReader stdInput = new BufferedReader(new InputStreamReader(process.getInputStream()));
					while ( (temp = stdInput.readLine()) != null ) {
						if ( temp.contains(GOGOC_BIN) ) {
							Log.d(LOG_TAG, "statusGogoc() temp='"+temp+"'");
							String [] cmdArray = temp.split(" +");
							for (i=0; i< cmdArray.length; i++) {
								Log.d(LOG_TAG, "loop i="+ i + " => " + cmdArray[i]);
							}
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
				
				// save conf in file
				saveConf();
				// load ipv6 and tun modules
				loadModule();
				
			    // if pid
			    if ( pid != "") {
			    	showMsg(R.string.gogoc_already_running);
			    }
			    else {
			    	startGogoc();
					Log.d(LOG_TAG, "onCreate() gogoc started.");
					changeStatus();
					if (statusGogoc()) {
						showMsg(R.string.gogoc_started);
					}
					setResult(android.app.Activity.RESULT_OK);
			    }


			}

		});
		
		
		btnStop = (Button) findViewById(R.id.ButtonStop);
		btnStop.setOnClickListener(new OnClickListener() {

			public void onClick(View v) {
				if ( stopGogoc() ){
					showMsg(R.string.gogoc_stopped);
					setResult(android.app.Activity.RESULT_OK);
				}
				changeStatus();
			}

		});
		
		btnDNS = (Button) findViewById(R.id.ButtonDNS);
		btnDNS.setOnClickListener(new OnClickListener() {

			public void onClick(View v) {
				setDNS();
				showMsg(R.string.dns_changed);
			}

		});
		
		
		//install gogoc binary
		installBinary();
		
		// load configuration in txtBox
		txtBox.setText( loadConf().toString() );
		
		// change checkbox status
		changeStatus();
    }
    
    
    public void changeStatus() {
    	if ( statusGogoc() ) {
			CheckboxRunning.setChecked(true);
			txtBox.setFocusable(false);
		}
		else {
			CheckboxRunning.setChecked(false);
			txtBox.setFocusable(true);
		}
    	
    }
    
    
    @SuppressWarnings("static-access")
	public void startGogoc() {
    	if( ! new File(GOGOC_BIN).exists() ) {
			showMsg(R.string.binary_not_exists);
			return;
		}
    	
		Thread thread = new Thread() {
			@Override
			public void run() {
				try {
		    		process = Runtime.getRuntime().exec("su -c sh");
		    		//process = Runtime.getRuntime().exec("sh");
		    		OutputStream os = process.getOutputStream();
		    		Log.d(LOG_TAG, "startGogoc() cmd="+GOGOC_BIN +" -y -f " +  GOGOC_CONF+"");
		    		writeLine( os, GOGOC_BIN + " -y -f " +  GOGOC_CONF );
		    		os.flush();
		    		//process.waitFor();
				}
				catch ( IOException e ) {
		    		e.printStackTrace();
		    	}
		    	/*catch (InterruptedException e) {
					e.printStackTrace();
				}*/
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
    
    
    public boolean stopGogoc() {
    	String pid;
    	String temp;
    	pid = "";
    	int i;
    	try{
    		Process p = Runtime.getRuntime().exec("ps");
			p.waitFor();
			
			BufferedReader stdInput = new BufferedReader(new InputStreamReader(p.getInputStream()));
			while ( (temp = stdInput.readLine()) != null ) {
				//Log.d(LOG_TAG, "stopGogoc() temp='"+temp+"'");
				if ( temp.contains(GOGOC_BIN) ) {
					Log.d(LOG_TAG, "statusGogoc() temp='"+temp+"'");
					String [] cmdArray = temp.split(" +");
					for (i=0; i< cmdArray.length; i++) {
						Log.d(LOG_TAG, "loop i="+ i + " => " + cmdArray[i]);
					}
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
	    
	    // if pid
	    if ( pid != "") {
	    	Log.d(LOG_TAG, "statusGogoc() killing='"+pid+"' ...");
	    	try {
		    	process = Runtime.getRuntime().exec("su -c sh");
		    	//process = Runtime.getRuntime().exec("sh");
				OutputStream os = process.getOutputStream();
				writeLine( os, "kill -9 " + pid); os.flush();
				writeLine( os, "exit \n"); os.flush();
				process.waitFor();
				return true;
	    	}
	    	catch (IOException e) {
				e.printStackTrace();
			}
		    catch (InterruptedException e) {
				e.printStackTrace();
			}
	    }
	    Log.d(LOG_TAG, "statusGogoc() pid empty='"+pid+"'");
	    return false;
    }

    
    public boolean statusGogoc() {
    	boolean run;
    	String temp;
    	
    	run=false;
    	Log.d(LOG_TAG, "statusGogoc() init");
    	
    	try {
			Process p = Runtime.getRuntime().exec("ps");
			p.waitFor();
			
			BufferedReader stdInput = new BufferedReader(new InputStreamReader(p.getInputStream()));
			while ( (temp = stdInput.readLine()) != null ) {
				if ( temp.contains(GOGOC_BIN) ) {
					Log.d(LOG_TAG, "statusGogoc() temp='"+temp+"'");
					run = true;
				}
			}
	    }
	    catch (IOException e) {
			e.printStackTrace();
		}
	    catch (InterruptedException e) {
			e.printStackTrace();
		}
    	return run;
    }
    
    
	public String loadConf() {
		String Script="";
		File gogoc_conf_folder = new File(GOGOC_DIR);
		
		if(!gogoc_conf_folder.exists())
		{
			Log.d(LOG_TAG, "Creating "+GOGOC_DIR+" folder");
			gogoc_conf_folder.mkdir();
		}
		else {
			Log.d(LOG_TAG, GOGOC_DIR+" exists");
		}
		
		// create gogoc.conf
		if( ! new File(GOGOC_CONF).exists() ) {
			showMsg(R.string.conf_not_exists);
			for (int i=0; i< DEFAULT_CONF.length; i++){
				Script += DEFAULT_CONF[i] + "\n";
			}
			return Script;
		}
		
		
		try {
	        BufferedReader in = new BufferedReader(new FileReader(GOGOC_CONF));
	        String str;
	        
	        while ((str = in.readLine()) != null) {
	        	Script += str + "\n";
	        }
	        in.close();
		}
		catch (Exception ex) {
			showMsg(R.string.cant_read_conf);
			return Script;
		}
		
		return Script;
	}
	
	
	public void loadModule(){
		File tundev = new File (TUNDEV);
		File if_inet6 = new File (IF_INET6);
		try {
    		process = Runtime.getRuntime().exec("su -c sh");
    		//process = Runtime.getRuntime().exec("sh");
    		OutputStream os = process.getOutputStream();
    		//check if /dev/tun exists
    		if ( ! tundev.exists()){
    			Log.d(LOG_TAG, "loadModule() cmd='modprobe tun'");
    			writeLine( os, "modprobe tun");
    		}
    		//check if /proc/net/if_inet6 exists
    		if ( ! if_inet6.exists()){
    			Log.d(LOG_TAG, "loadModule() cmd='modprobe ipv6");
    			writeLine( os, "modprobe ipv6");
    		}
    		os.flush();
    		//process.waitFor();
		}
		catch ( IOException e ) {
    		e.printStackTrace();
    	}
    	/*catch (InterruptedException e) {
			e.printStackTrace();
		}*/
	}
	
	
	public void setDNS(){
		try {
    		process = Runtime.getRuntime().exec("su -c sh");
    		//process = Runtime.getRuntime().exec("sh");
    		OutputStream os = process.getOutputStream();
    		Log.d(LOG_TAG, "setDNS() cmd=setprop net.dns1 " + DNS1 );
    		writeLine( os, "setprop net.dns1 " + DNS1 );
    		os.flush();
    		//process.waitFor();
		}
		catch ( IOException e ) {
    		e.printStackTrace();
    	}
    	/*catch (InterruptedException e) {
			e.printStackTrace();
		}*/
	}
	
	
	public void installBinary() {
		File gogoc_working_folder = new File(GOGOC_WORKING_DIR);
		File gogoc_binary = new File(GOGOC_BIN);
		
		// create gogoc working directory
		if(!gogoc_working_folder.exists())
		{
			Log.d(LOG_TAG, "Creating "+GOGOC_WORKING_DIR+" folder");
			gogoc_working_folder.mkdir();
		}
		
		// install gogoc binary
		if(!gogoc_binary.exists())
		{
			copyRaw(R.raw.gogoc, (GOGOC_BIN));
			changePermission();
			showMsg(R.string.binary_installed);
		}
	}
		
		
	public void changePermission(){
		try {
			process = Runtime.getRuntime().exec("su -c sh");

			OutputStream os = process.getOutputStream();
			writeLine(os, "chmod 755 " + (GOGOC_BIN));
			os.close();
		} catch (IOException e) {
			Log.d(LOG_TAG, "Error IOException");	
		}catch (Exception e) {
			Log.d(LOG_TAG, "Error Exception");		
		}
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
		catch (Exception e)
		{
			Log.d(LOG_TAG, "Installing gogoc binary... ");
		}
	}
	
	
	public void saveConf() {
		Writer output = null;
		Log.d(LOG_TAG, "saveConf() init");
	    try {
	    	output = new BufferedWriter(new FileWriter(GOGOC_CONF));
	    	output.write( txtBox.getText().toString() );
	    	output.close();
	    	Log.d(LOG_TAG, "saveConf() saved and closed");
	    }
	    catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	
	public void showMsg(int txt) {
		int text = txt;
		int duration = Toast.LENGTH_LONG;
		Toast toast = Toast.makeText(this, text, duration);
		toast.show();
		Log.d(LOG_TAG, "showMsg() txt='"+txt+"'");
	}
	
	
	public static void writeLine(OutputStream os, String value) throws IOException
	{
		String line = value + "\n";
		os.write( line.getBytes() );
	}
	
  
}


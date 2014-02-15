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
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.EditText;
import android.widget.TextView;

import com.googlecode.gogodroid.R;

public class GogoDroid extends Activity {
	
	RadioButton StatusRunning;
	Button btnStartStop;
	EditText gogocConfig;
	EditText currentIP;
	TextView conftxt;
  GogoCtl ctl;
	
    /** Called when the activity is first created. */
	    @Override
	    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        StatusRunning = (RadioButton) findViewById(R.id.Running);
        gogocConfig = (EditText) findViewById(R.id.GogocConf);
        currentIP = (EditText) findViewById(R.id.Address);
        conftxt = (TextView) findViewById(R.id.ConfText);

        
		btnStartStop = (Button) findViewById(R.id.ButtonStartStop);
		btnStartStop.setOnClickListener(new OnClickListener() {
		
			public void onClick(View v) {
				
      if( !ctl.statusGogoc()) {
				// save conf in file
				ctl.saveConf( gogocConfig.getText().toString() );
				ctl.startGogoc();
				ctl.showToast(R.string.gogoc_started);
				Log.d(Constants.LOG_TAG, "onCreate() Gogoc started.");
				/*try {
          // TODO: move to a service ?
					while(true){
						Thread.sleep(2000);
						if(statusConnection()=="established")
							break;
						}
				}
				catch (Exception e) {
					e.printStackTrace();
				}*/
				setResult(android.app.Activity.RESULT_OK);
        ((Button) v).setText(R.string.btn_stop);
			}
      else {
				ctl.stopGogoc();
				statusConnection();
				ctl.showToast(R.string.gogoc_stopped);
				setResult(android.app.Activity.RESULT_OK);
        ((Button) v).setText(R.string.btn_start);
      }
        }
			});
		
		
    // Create controller instance:
    ctl = new GogoCtl(this);

    // Initialize controller:
    ctl.init();
		
    if( ctl.statusGogoc()) {
      btnStartStop.setText(R.string.btn_stop);
    }
    else {
      btnStartStop.setText(R.string.btn_start);
    }

		// load configuration in gogocConfig
		gogocConfig.setText( ctl.loadConf().toString() );
		
		// save configuration
		ctl.saveConf(gogocConfig.getText().toString());
		
		// check gogodroid status
		statusConnection();

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        MenuItem startStopItem = menu.findItem(R.id.start_stop);
        if( ctl.statusGogoc()) {
          startStopItem.setTitle(R.string.btn_stop);
        }
        else {
          startStopItem.setTitle(R.string.btn_start);
        }
        return true;
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch(item.getItemId()) {
          case R.id.action_preferences:
            startActivity(new Intent(this, GogoPreferenceActivity.class));
            return true;
          case R.id.action_exit:
          if ( ctl.statusGogoc()) {
              ctl.stopGogoc();
            }
            finish();
            return true;
          case R.id.set_dns:
            setDNS();
            ctl.showToast(R.string.dns_changed);
            return true;
          case R.id.start_stop:
            //item.setTitle(R.string.btn_stop);
            return true;
          default:
            return super.onOptionsItemSelected(item);
        }

    }


    @SuppressWarnings("static-access")
	public void setDNS(){
    	
		Thread thread = new Thread() {
			@Override
			public void run() {
	    		Utils.runSuCommand("setprop net.dns1 " + Constants.DNS1 );
			}
		};
		thread.start();
		// sleep to give some time to statusGogoc to detect process
		try{
		  Thread.currentThread().sleep(1000);//sleep for 1000 ms
		}
		catch(Exception e){
			e.printStackTrace();
		}

	}

	
	public String statusConnection() {
		String linkstatus;
		linkstatus = "not_available";
		showIndicator("not_available");
		if (ctl.statusGogoc())  {
			linkstatus = "connecting";
			showIndicator("connecting");
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
						currentIP.setText(stringbuilder.toString()
							.replaceAll(":(0000)+", ":")
							.replaceFirst("::+", "::"));
						linkstatus = "established";
						showIndicator("established");
						Log.d(Constants.LOG_TAG, "statusConnection() address=" + stringbuilder.toString());
						break;
					}
				}
				bufferedreader.close();
			}
			catch (Exception e) {
				linkstatus = "error";
				showIndicator("error");
			}
		}
		Log.d(Constants.LOG_TAG, "statusConnection() status=" + linkstatus);
		return linkstatus;
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

}

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

import java.io.IOException;
import java.io.OutputStream;

import android.util.Log;

public class Utils {
    
	private static final String LOG_TAG = "Gogoc";


    public static void runCommand(String command) {
    	try {
    		Process process = Runtime.getRuntime().exec("sh");
    		//process = Runtime.getRuntime().exec("sh");
    		OutputStream os = process.getOutputStream();
    		Log.d(LOG_TAG, "startGogoc() cmd=" + command );
    		writeLine( os, command );
    		os.flush();
    		}
    	catch ( IOException e ) {
    		e.printStackTrace();
    	}
    }


    public static void runSuCommand(String sucommand) {
    	try {
    		Process process = Runtime.getRuntime().exec("su -c sh");
    		//process = Runtime.getRuntime().exec("sh");
    		OutputStream os = process.getOutputStream();
    		Log.d(LOG_TAG, "startGogoc() rootcmd=" + sucommand );
    		writeLine( os, sucommand );
    		os.flush();
    		}
    	catch ( IOException e ) {
    		e.printStackTrace();
    	}
    }
    
    
	private static void writeLine(OutputStream os, String value) throws IOException
	{
		String line = value + "\n";
		os.write( line.getBytes() );
	}
}
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

import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;

public class GogoPreferenceActivity extends PreferenceActivity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
    getPreferenceManager().setSharedPreferencesName(Constants.PREFS_FILE);
		addPreferencesFromResource(R.xml.preferences);

    findPreference("auth_method").setOnPreferenceChangeListener(
      new Preference.OnPreferenceChangeListener() {
        public boolean onPreferenceChange(Preference preference, Object newValue) {
          findPreference("userid").setEnabled(!((String) newValue).equals("anonymous"));
          findPreference("passwd").setEnabled(!((String) newValue).equals("anonymous"));
          return(true);
        }
    });
	}
}

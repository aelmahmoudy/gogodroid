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

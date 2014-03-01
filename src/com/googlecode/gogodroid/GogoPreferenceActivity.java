package com.googlecode.gogodroid;

import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;

public class GogoPreferenceActivity extends PreferenceActivity implements OnSharedPreferenceChangeListener {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
    getPreferenceManager().setSharedPreferencesName(Constants.PREFS_FILE);
		addPreferencesFromResource(R.xml.preferences);
    getPreferenceManager().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);
	}

  @Override
  public void onSharedPreferenceChanged(final SharedPreferences sharedPreferences, final String key) {
    if(key.equals("auth_method")) {
      findPreference("userid").setEnabled(!sharedPreferences.getString("auth_method", "anonymous").equals("anonymous"));
      findPreference("passwd").setEnabled(!sharedPreferences.getString("auth_method", "anonymous").equals("anonymous"));
    }
  }

/*	@Override
	protected void onDestroy() {
		super.onDestroy();
	}*/

}

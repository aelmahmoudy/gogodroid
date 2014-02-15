package com.googlecode.gogodroid;

import android.os.Bundle;
import android.preference.PreferenceActivity;

public class GogoPreferenceActivity extends PreferenceActivity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.preferences);
	}

/*	@Override
	protected void onDestroy() {
		super.onDestroy();
	}*/

}

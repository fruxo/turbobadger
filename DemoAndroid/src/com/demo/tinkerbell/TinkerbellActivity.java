package com.demo.tinkerbell;

import android.app.Activity;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;


public class TinkerbellActivity extends Activity
{
	static AssetManager sAssetManager;
	TinkerbellView mView;

	// On applications creation
	@Override protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		// Pass the asset manager to the native code
		sAssetManager = getAssets();
		TinkerbellLib.createAssetManager(sAssetManager);

		// Create our view for OpenGL rendering
		mView = new TinkerbellView(getApplication());

		setContentView(mView);
	}

	@Override protected void onPause()
	{
		super.onPause();
		mView.onPause();
	}

	@Override protected void onResume()
	{
		super.onResume();
		mView.onResume();
	}

	@Override public void onBackPressed ()
	{
		// FIX: Queue event to correct thread but wait for return value!
		// if (TinkerbellLib.OnBackKey() == 0)
		super.onBackPressed();
	}

	@Override public boolean onCreateOptionsMenu(Menu menu)
	{
		/*if (mInitiated)
		{
			mView.queueEvent(new Runnable() {
				public void run() {
					TinkerbellLib.OnMenuKey();
				}});
		}
		return false; */
		menu.add("Exit");
		return super.onCreateOptionsMenu(menu);
	}

	@Override public boolean onOptionsItemSelected(MenuItem item)
	{
		// Exit
		mView.queueEvent(new Runnable() {
			public void run() {
				TinkerbellLib.ShutDownApp();
			}});
		return false;
	}
}

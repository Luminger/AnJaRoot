package org.failedprojects.anjaroot;

import android.os.Bundle;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.support.v4.app.FragmentActivity;
import android.view.Menu;

public class MainActivity extends FragmentActivity {
	
	static final String LOGTAG = "AnjaRoot";
	
	ProgressDialog createProgessDialog() {
		ProgressDialog dial = new ProgressDialog(this);
		dial.setTitle("AnJaRoot");
		dial.setMessage("Testing root access...");
		dial.setIndeterminate(true);
		dial.show();
		
		return dial;
	}
	
	void doSystemInstall() {
		final ProgressDialog dial = createProgessDialog();
		new Thread() {
			public void run() {
				dial.dismiss();
			}
		}.run();
	}
	
	void doRecoveryInstall() {
		
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setTitle("AnJaRoot");
		builder.setMessage("Do you want to install AnJaRoot?");
		builder.setPositiveButton("Install", new OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				doSystemInstall();
			}
		});
		builder.setNeutralButton("Recovery", new OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				doRecoveryInstall();
			}
		});
		builder.setNegativeButton("Cancel", null);
		builder.create().show();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

}

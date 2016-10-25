//Author: Luan Pham

package com.example.draw;

import android.app.Activity;
import android.os.Bundle;
import android.view.Display;
import android.view.Window;
import android.view.WindowManager;


// this is an activity which create a drawview and record it
public class MainActivity extends Activity  {
   
	DrawView drawView;	// paint view
    record rec;			// class that record touch event of a view
    
    // this method is create when android app is first startup
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);		// save instant of this activity
        // Set full screen view
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, 
                                         WindowManager.LayoutParams.FLAG_FULLSCREEN);
        requestWindowFeature(Window.FEATURE_NO_TITLE);	// no tilte on app

        Display display = getWindowManager().getDefaultDisplay();
        drawView = new DrawView(this);  // create paint view
       // drawView.requestFocus();
        rec = new record(this,drawView,display.getWidth(), display.getHeight() ); //pass on view and record it
       
        setContentView(rec);				// set view to rec
       
             
    }
    
   
}
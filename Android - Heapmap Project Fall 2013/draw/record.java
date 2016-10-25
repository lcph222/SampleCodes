//Author: Luan Pham (Android)


package com.example.draw;

import java.io.File;
import java.util.ArrayList;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import com.example.draw.FileUtility;
import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Handler;
import android.provider.Settings.Secure;
import android.view.MotionEvent;
import android.widget.LinearLayout;

//this class has method to record a view, parse the data and send it to a server. It can also get the data from 
// a server and deserialized it and replay the user touch events. 
@SuppressLint("Recycle")
public class record extends LinearLayout implements APIDelegate{
	
	ArrayList<MotionEvent> EventList;	// stored all the touch events 
	DrawView view;					    // we dont' need this, but just stored the view of the child(paint program)
	JSONArray JsonList;					// list of json objects
	String[] Jstring;				
	private int screen_height, screen_width; // device screen height and width
	private boolean start = false; 		// boolean used to start and stop recording
    private boolean firstTouch = true;	// boolean used to start recording on first touch
	
	private int i = 0;							// position of motionData
	private long dx = 0;						//1000 millisecond delay to replay
	private Handler handler = new Handler();	//new thread
	private Runnable runnable; 					// function add to handler
	
	Handler handler2 = new Handler();			// new thread
	Runnable runnable2; 						// new runable
	boolean go = true;							// boolean to start recording
	
	
	// constructor, initialized variable. this function take a context and a view
	public record(Context context, DrawView view, int width, int height) {
		super(context);							// pass the context to the superclass
		this.addView(view);						// add view to viewgroup
		this.view = view;						// save childview used for debugging
		EventList = new ArrayList<MotionEvent>();	// allocate space for list of motion event object
		JsonList = new JSONArray();					// allocate space for jsonList
		screen_height = height; 
		screen_width = width;

	}

	// this function is auto generated and it's important when implement a viewgroup
	@Override
	protected void onLayout(boolean changed, int l, int t, int r, int b) {
		// TODO Auto-generated method stub
		 super.onLayout(changed, l, t, r, b);
	}

	
	// this method is used to record all the touch point of the childview before pass the event
	// to child view
    @Override
	public boolean onInterceptTouchEvent (MotionEvent ev){

    // start recording 
    if(firstTouch){
    	firstTouch = false;
    	execute();}	
    	
	// start = true, start recording
	if(start){			
	    MotionEvent tempEv = MotionEvent.obtain(ev);	//copy event
		EventList.add(tempEv);
		}							// save it		
	
		return false;									// pass the touch event to the child view(paint)
    }
	
	public void startRecording(){		// clear data and set start to record
		EventList.clear();
		start = true;
	}
	public void stopRecording(){		// set start to false, stop recording
		start = false;
	}

	// convert a list of motionEvent object to a list of json object so we can save it to a file and send it to a server
	public boolean setJson(){
		// this function return false if there is an error in converting a list of motionevent to a list of JsonObject
		
		try {
			// set device info in json file 
			final String androidId = Secure.getString(this.getContext().getContentResolver(), Secure.ANDROID_ID);
		    String APIKEY = "placeholder";
		    JSONObject jsonObj1 = new JSONObject();
		    jsonObj1.put("APIKEY", APIKEY);
		    jsonObj1.put("screen width", screen_width);      jsonObj1.put("screen height", screen_height); 
		    jsonObj1.put("Serial", androidId);
		    JsonList.put(jsonObj1);

		//iterate through the list
		for(MotionEvent e:EventList){
			
			// create a temp jsonObj and initialize all the value of the motionEvent objects 
			// an event can be retrieve if we have all the below value
			JSONObject jsonObj = new JSONObject();
			
			jsonObj.put("eventTime", e.getEventTime()); 	jsonObj.put("downTime", e.getDownTime());
			jsonObj.put("action", e.getAction());			jsonObj.put("deviceId", e.getDeviceId());
			jsonObj.put("edgeFlags", e.getEdgeFlags()); 	jsonObj.put("metaState",e.getMetaState());
			jsonObj.put("x", e.getX());
			jsonObj.put("y", e.getY());						jsonObj.put("size", e.getSize());
			jsonObj.put("xPrecision", e.getXPrecision());	jsonObj.put("yPrecision", e.getYPrecision());
			jsonObj.put("pressure", e.getPressure());
			
			// add the jsonObj to a Json List
			JsonList.put(jsonObj);	
		}
		
		} catch (JSONException e) {
			// return false if there's an error
			return false;
		}
		
		return true;
	}
	
	// 
	JSONArray jArray;
	
	// this function will save the MotionEVent List to a file locally 
	public boolean save() {
	
//---->		//note this function should return boolean so check/update later
		if(!setJson())
			return false;
		// file name including directory
		String filename = "directory"+File.separator+"testfile2.txt";
		String output =null;
		// output is a json string which represents the data of all the user touch points 
		// created in setJson function
		output = JsonList.toString();
		// write to data using fileUtility class given to us by chris allen
		FileUtility.writeToFile(this.getContext(), output, filename); // <----check for return value for error handling
		
		// return true
		return true;
	}
	
	// this method will load the data in from a textfile and parse it back to a List of MotionEVent obj
	// return false if there's an error, otherwise true
	public boolean loadData(){
		
		EventList.clear();	// clear array of motionEvent
		// filename
		String filename = "directory"+File.separator+"testfile2.txt";
		// open and read in file as a string and stored in fileContents
		String fileContents = FileUtility.readFile(this.getContext(), filename);
	
		// these are the values of MotionEvent object that we need before we can 
		// retrieved a MotionEvent object
		long downTime, eventTime;
		int action, metaState,deviceId,edgeFlags;
		float x, y, pressure, size, xPrecision,yPrecision;
		
		 try {
			
			//JArray is a JsonArray String(MotionEVent string)
			jArray = new JSONArray(fileContents);

			for(int i = 0; i < jArray.length();i++){
				
				// get the json object
				JSONObject j = jArray.getJSONObject(i);
				
				// get the values from json object
				downTime = j.getLong("downTime"); 
				eventTime = j.getLong("eventTime");
				action = j.getInt("action");
				x = (float) j.getLong("x");
				y = (float) j.getLong("y");
				pressure = (float) j.getLong("pressure");
				size = (float) j.getLong("size");
				metaState = (int) j.getInt("metaState");
				xPrecision = (float) j.getLong("xPrecision");
				yPrecision = (float) j.getLong("yPrecision");
				deviceId = j.getInt("deviceId");
				edgeFlags = j.getInt("edgeFlags");
				
				//recreate the event using the value above
				MotionEvent tempEv = MotionEvent.obtain(downTime, eventTime, action, x, y, pressure, size, metaState,
														xPrecision, yPrecision, deviceId, edgeFlags);
				
				// add the event to a list
				EventList.add(tempEv);
			}
		} catch (JSONException e) {
			//error
			return false;
		}
		
		 // return true if there's no error
		 return true;
	}
	
	// this function will replay all the touch events by passing the event to the childView
	public void play(){	
			i = 0;
			// a runnable that will call sendMsg and increment i
		    runnable = new Runnable() {
		        public void run() {
		            sendMsg(i);				// i is the postion of motionData (List of event) to replay
		            i++;					// next event
		        }
		    };

		    runnable.run();   
		
	}
	
	// this function will send event to the childview 
	public void sendMsg(int i){
		// loop through event list that was recorded
		if(i < EventList.size()){
			handler.postDelayed(runnable, dx);	// update handler with delay time, which is dx
			if(i < (EventList.size() -1) )		// calculate the next delay time dx...for example the time between 1st and 2nd touch
				dx = EventList.get(i + 1).getEventTime() - EventList.get(i).getEventTime();
			this.getChildAt(0).dispatchTouchEvent(EventList.get(i));		// send event to the child view
			}
		else
			handler.removeCallbacks(runnable);	// remove runnable, which is a thread that called this function
		
	}

	
	// this function start everything, e.g. start recording
	public void execute(){
		
		 runnable2 = new Runnable() {
		        public void run() {
		        		startTest();
		        }
		    };

		    runnable2.run();   

	}
	
	//this function get and post data
	// this function will be motified soon
	// basictly it used for testing 
	public void request () {		
		
		//String urlStr = "http://echo.jsontest.com/insert-key-here/insert-value-here/key/value";
		serverAPI s;			// a class that handle post and get
		s = new serverAPI();	
		s.setDelegate(this);	//set delegate to this 
		//s.get();
		final String smg = JsonList.toString(); // convert json List to string
		s.post(smg);							// post the data to a server
	}
	
	
	// this function set recording time to 20000 mms(20 seconds)
	// this function is for testing
	public void startTest(){

			if(go){
	    		startRecording();			// start recording
	    		handler2.postDelayed(runnable2, 5000);	// record for 5 seconds
	    		}
	    	else {
	    		stopRecording();			// stop recording
	    		handler2.removeCallbacks(runnable2);// remove runnable from handler
	    		view.points.clear();				// clear point program
	    		
	    		save();								// save data
	    		if(loadData())						// load data from file
	    			play();							// replay it
	    		request(); // sending file and get file
	    	}
	    	go = !go;	// stop recording 
	    	
	    }

	// these method is called when get return
	@Override
	public void getReturned(serverAPI api, String result, Exception e) {
		// TODO Auto-generated method stub
		System.out.println(result);
		
	}

	@Override
	public void postReturned(serverAPI api, boolean success, Exception e) {
		// TODO Auto-generated method stub
		System.out.println(success);
	}


}

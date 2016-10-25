//Author: Joshua Yancy

package com.example.draw;

// this is an interface that handle post and get return

public interface APIDelegate {
	// handle get response
	public void getReturned(serverAPI api, String result, Exception e);
	// handle post response
	public void postReturned(serverAPI api, boolean success, Exception e);
}

package com.android.o3djni;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;

import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.util.Log;

public class WebServer implements Runnable {
	private int mPort = 4444;
	private Map<String, String> mFileCache = new HashMap<String, String>();
	private AssetManager mAssetManager;

	WebServer(AssetManager assets) {
		mAssetManager = assets;
	}
	
	// based heavily on http://fragments.turtlemeat.com/javawebserver.php
	public void run() {
		//we are now inside our own thread separated from the gui.
	    ServerSocket serversocket = null;
	    
	    Log.d("O3D Web", "Opening Connection on Port " + mPort + ", ip = " + getLocalIpAddress());
	    try {
	    	serversocket = new ServerSocket(mPort);
	    }
	    catch (Exception e) { 
			Log.d("O3D Web", "Error: " + e.getMessage());
			return;
	    }
	    
	    while (true) {
	      try {
	        //this call waits/blocks until someone connects to the port we
	        //are listening to
	    	Log.d("O3D Web", "Waiting for connection...");
	        Socket connectionsocket = serversocket.accept();
	        //figure out what ipaddress the client comes from, just for show!
	        InetAddress client = connectionsocket.getInetAddress();
	        Log.d("O3D Web", "Connection request from " + client.getHostName());
	        //Read the http request from the client from the socket interface
	        //into a buffer.
	        BufferedReader input =
	            new BufferedReader(new InputStreamReader(connectionsocket.
	            getInputStream()));
	        //Prepare a outputstream from us to the client,
	        //this will be used sending back our response
	        //(header + requested file) to the client.
	        DataOutputStream output =
	            new DataOutputStream(connectionsocket.getOutputStream());
	
	        //as the name suggest this method handles the http request, see further down.
	        //abstraction rules
	        http_handler(input, output);
	      }
	      catch (Exception e) { //catch any errors, and print them
	        Log.d("O3D Web", "Error: " + e.getMessage());
	      }
	
	    } //go back in loop, wait for next request
	    
	}
	
	
	private void http_handler(BufferedReader input, DataOutputStream output) {
	    int method = 0; //1 get, 2 head, 0 not supported
	    String http = new String(); //a bunch of strings to hold
	    String path = new String(); //the various things, what http v, what path,
	    String file = new String(); //what file
	    String user_agent = new String(); //what user_agent
	    try {
	      //This is the two types of request we can handle
	      //GET /index.html HTTP/1.0
	      //HEAD /index.html HTTP/1.0
	      String tmp = input.readLine(); //read from the stream
	      String tmp2 = new String(tmp);
	      tmp.toUpperCase(); //convert it to uppercase
	      if (tmp.startsWith("GET")) { //compare it is it GET
	        method = 1;
	      } //if we set it to method 1
	      if (tmp.startsWith("HEAD")) { //same here is it HEAD
	        method = 2;
	      } //set method to 2
	
	      if (method == 0) { // not supported
	        try {
	          output.writeBytes(construct_http_header(501, 0));
	          output.close();
	          return;
	        }
	        catch (Exception e3) { //if some error happened catch it
	        	Log.d("O3D Web", "HTTP Error: " + e3.getMessage());
	        } //and display error
	      }
	      
	
	      //tmp contains "GET /index.html HTTP/1.0 ......."
	      //find first space
	      //find next space
	      //copy whats between minus slash, then you get "index.html"
	      //it's a bit of dirty code, but bear with me...
	      int start = 0;
	      int end = 0;
	      for (int a = 0; a < tmp2.length(); a++) {
	        if (tmp2.charAt(a) == ' ' && start != 0) {
	          end = a;
	          break;
	        }
	        if (tmp2.charAt(a) == ' ' && start == 0) {
	          start = a;
	        }
	      }
	      path = tmp2.substring(start + 2, end); //fill in the path
	    }
	    catch (Exception e) {
	    	Log.d("O3D Web", "Http Error: " + e.getMessage());
	    } //catch any exception
	
	    String response = parseRequest(path);
	    
	    if (response.length() == 0) {
	        //if you could not open the file send a 404
	        try {
				output.writeBytes(construct_http_header(404, 0));
				//close the stream
		        output.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
	        
	    } else {
	      int type_is = 0;
	      //find out what the filename ends with,
	      //so you can construct a the right content type
	      if (path.endsWith(".zip") || path.endsWith(".exe")
	          || path.endsWith(".tar")) {
	        type_is = 3;
	      }
	      if (path.endsWith(".jpg") || path.endsWith(".jpeg")) {
	        type_is = 1;
	      }
	      if (path.endsWith(".gif")) {
	        type_is = 2;
	        //write out the header, 200 ->everything is ok we are all happy.
	      }
	      try {
			output.writeBytes(construct_http_header(200, 5));

			//if it was a HEAD request, we don't print any BODY
			if (method == 1) { //1 is GET 2 is head and skips the body
			    output.writeBytes(response);
			}
			//clean up the files, close open handles
			output.close();
	      } catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
	      }
	    }
	}

  //this method makes the HTTP header for the response
  //the headers job is to tell the browser the result of the request
  //among if it was successful or not.
  private String construct_http_header(int return_code, int file_type) {
    String s = "HTTP/1.0 ";
    //you probably have seen these if you have been surfing the web a while
    switch (return_code) {
      case 200:
        s = s + "200 OK";
        break;
      case 400:
        s = s + "400 Bad Request";
        break;
      case 403:
        s = s + "403 Forbidden";
        break;
      case 404:
        s = s + "404 Not Found";
        break;
      case 500:
        s = s + "500 Internal Server Error";
        break;
      case 501:
        s = s + "501 Not Implemented";
        break;
    }

    s = s + "\r\n"; //other header fields,
    s = s + "Connection: close\r\n"; //we can't handle persistent connections
    s = s + "Server: SimpleHTTPtutorial v0\r\n"; //server name

    //Construct the right Content-Type for the header.
    //This is so the browser knows what to do with the
    //file, you may know the browser dosen't look on the file
    //extension, it is the servers job to let the browser know
    //what kind of file is being transmitted. You may have experienced
    //if the server is miss configured it may result in
    //pictures displayed as text!
    switch (file_type) {
      //plenty of types for you to fill in
      case 0:
        break;
      case 1:
        s = s + "Content-Type: image/jpeg\r\n";
        break;
      case 2:
        s = s + "Content-Type: image/gif\r\n";
      case 3:
        s = s + "Content-Type: application/x-zip-compressed\r\n";
      default:
        s = s + "Content-Type: text/html\r\n";
        break;
    }

    ////so on and so on......
    s = s + "\r\n"; //this marks the end of the httpheader
    //and the start of the body
    //ok return our newly created header!
    return s;
  }
  
  private String getLocalIpAddress() {
    try {
    	for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();) {
    		NetworkInterface intf = en.nextElement();
	        for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();) {
	            InetAddress inetAddress = enumIpAddr.nextElement();
	            if (!inetAddress.isLoopbackAddress()) {
	                return inetAddress.getHostAddress().toString();
	            }
	        }
	    }
	} catch (SocketException ex) {
	    Log.e("O3D Web", ex.toString());
	}
	return null;
  }

	private static String formatLink(String s, String path) {
		return "<a href=\"" + path + s + "\">" + s + "</a>";
	}
  
	private String parseRequest(String path) {
		String response  = "";
		if (path.length() == 0) {
			response = readFile("client.html");
		} else {
			StringBuilder buffer = new StringBuilder();
			String fullPath = "/" + path + "/";
			String[] parts = path.split("/");
			if (parts.length == 1) {
				String[] result = O3DJNILib.getSystemList();
				for (int x = 0; x < result.length; x++) {
					buffer.append(formatLink(result[x], fullPath));
					buffer.append("<br>\n");
				}
			} else {
				String[][] result = O3DJNILib.getMetaData(parts); 
				if (result != null) {
					buffer.append("<table border='0'>");
					for (int x = 0; x < result.length; x++) {
						buffer.append("<tr><td>");
						buffer.append(result[x][1]);
						buffer.append("</td><td>");
						buffer.append(formatLink(result[x][0], fullPath));
						buffer.append("</td><td>");
						buffer.append(result[x][2]);
						buffer.append("</td></tr>\n");
					}
					buffer.append("</table>");
				} else {
					buffer.append("(no fields)");
				}
			}
			response = buffer.toString();
		}
		
		return response;
	}
	
	private String readFile(String path) {
		String result = "";
		if (mFileCache.containsKey(path)) {
			result = mFileCache.get(path);
			Log.i("O3D Web", "Pulled " + path + " from cache.");
		} else {
			// Read the file and cache it.
			try {
				InputStream inputStream = mAssetManager.open(path);
				ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
				byte buf[] = new byte[1024];
				int len;
			    while ((len = inputStream.read(buf)) != -1) {
			        outputStream.write(buf, 0, len);
			    }
			    outputStream.close();
			    inputStream.close();
			    
			    result = outputStream.toString();
			    
			    mFileCache.put(path, result);

			} catch (IOException e) {
				Log.e("O3D Web", e.getMessage());
			}
		}
		return result;
	}
	
}


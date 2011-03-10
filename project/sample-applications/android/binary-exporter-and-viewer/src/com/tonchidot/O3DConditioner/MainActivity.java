/*
 * Copyright (C) 2010 Tonchidot Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.tonchidot.O3DConditioner;

import android.app.*;
import android.content.*;
import android.os.Bundle;
import android.util.Log;
import android.content.Intent;
import android.view.*;
import android.view.View.*;
import android.view.GestureDetector.*;
import android.widget.*;
import android.widget.SlidingDrawer.*;
import android.graphics.PixelFormat;
import roboguice.activity.*;
import roboguice.inject.*;
import java.io.*;
import java.util.*;

public class MainActivity extends RoboActivity {
	@InjectView(R.id.native_view)
  protected NativeView mNativeView;
  @InjectView(R.id.file_list)
  protected ListView  mListView;

  protected FileFilter mFileFilter = new FileFilter();
  protected File mRoot;
  protected File mCurrentDir;
  protected File mCurrentModel = null;
  protected List<File> mEntries = null;
  protected FileListAdapter mAdapter;
  protected ProgressDialog mWaiter;
  protected GestureDetector mGestureDetector;
  
	@Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    mRoot       = getExternalFilesDir(null);
    mCurrentDir = new File(mRoot, "/");
    mAdapter = new FileListAdapter(this);

    setContentView(R.layout.main);
    mNativeView.getHolder().setFormat(PixelFormat.TRANSLUCENT);
    //mNativeView.setDrawingCacheEnabled(true);
    //mNativeView.setDrawingCacheQuality(View.DRAWING_CACHE_QUALITY_HIGH);

    mGestureDetector = new GestureDetector(new SimpleOnGestureListener() {
      @Override
      public void onLongPress(MotionEvent e) {
        final File currentModel=MainActivity.this.mCurrentModel;

        // No model shown -> do nothing
        if (currentModel == null) {
          Log.i(O3DConditioner.TAG, "mCurrentModel is null");
          return;
        }

        // Collada model shown -> ask to convert
        if (currentModel.getName().toLowerCase().matches(".+\\.dae$")) {
          AlertDialog alert = new AlertDialog.Builder(MainActivity.this)
            .setMessage("Convert "+MainActivity.this.mCurrentModel.getName()+"?")
            .setCancelable(false)
            .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
              public void onClick(DialogInterface dialog, int id) {
                String path = null;
                try { path = currentModel.getCanonicalPath(); }
                catch(Exception wtf) { }
                if (path == null) return;
                path = path.substring(0,path.lastIndexOf('.')) + ".o3dbin"; 
                Log.i(O3DConditioner.TAG, "Exporting to ["+path+"]");
                mWaiter = ProgressDialog.show(MainActivity.this, "", "Please wait", true, false);
                mNativeView.exportScene(new File(path), new NativeView.OnCompleteListener() {
                  public void onComplete(boolean result) {
                    mWaiter.dismiss();
                  }
                });
              }
            })
            .setNegativeButton("No", new DialogInterface.OnClickListener() {
              public void onClick(DialogInterface dialog, int id) {
                dialog.cancel();
              }
            })
            .create();
          alert.show();
        }
      }
    });

    mNativeView.setZOrderOnTop(true);
    mNativeView.setOnTouchListener(new OnTouchListener() {
      public boolean onTouch(View v, MotionEvent event) {
        mGestureDetector.onTouchEvent(event);
        return true;
      }
    });
    mListView.setAdapter(mAdapter);
    mListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
      public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        mWaiter = ProgressDialog.show(MainActivity.this, "", "Please wait", true, false);
        
        final File selectedFile = mEntries.get(position);
        if (selectedFile.isDirectory()) {
          try {
            mCurrentDir = selectedFile;
            if ("..".equals(selectedFile.getName())) {
              // This is a workaround File's getCanonical{Path,File}() bug
              mCurrentDir = mCurrentDir.getParentFile(); // current dir
              mCurrentDir = mCurrentDir.getParentFile(); // parent dir
            }
            mCurrentDir = mCurrentDir.getCanonicalFile();
            Log.i(O3DConditioner.TAG, "Changing dir to ["+mCurrentDir.getCanonicalPath()+"]");
          }
          catch(IOException ignored) { }
          refreshList();
          mWaiter.dismiss();
        }
        else {
          mNativeView.loadScene(selectedFile, new NativeView.OnCompleteListener() {
            public void onComplete(boolean result) {
              mWaiter.dismiss();
              if (result) {
                Log.i(O3DConditioner.TAG, "mCurrentModel = "+selectedFile.getName());
                mCurrentModel = selectedFile;
              }
            }
          });
        }
      }
    });
  }

  @Override
  public void onAttachedToWindow() {
    super.onAttachedToWindow();
    getWindow().setFormat(PixelFormat.TRANSLUCENT);
  }

  @Override
  protected void onResume() {
    super.onResume();
    mNativeView.onResume();
    refreshList();
  }

  @Override
  protected void onPause() {
    super.onPause();
    mNativeView.onPause();
  }

  private void refreshList() {
    File[] files = mCurrentDir.listFiles(mFileFilter);
    try {
      if (!mCurrentDir.equals(mRoot)) {
        File[] tmp = new File[1+files.length];
        tmp[0] = new File(mCurrentDir, "..");
        System.arraycopy(files, 0, tmp, 1, files.length);
        files = tmp;
      }
    }
    catch(Exception wtf) { }
    mEntries = Arrays.asList(files);
    mAdapter.notifyDataSetChanged();
  }

  private static class FileFilter implements FilenameFilter {
      public boolean accept (File dir, String filename) {
          final File file = new File(dir, filename);
          final String lc=filename.toLowerCase();
          // Only accept .dae and .o3dbin files, or directories
          if (!(lc.matches(".+\\.(dae|o3dbin)$") || file.isDirectory())) {
            return false;
          }
          // Do not accept resource forks
          if (lc.matches("\\.[^\\\\.].+")) {
            return false;
          }
          return true;
      }
  }

  private class FileListAdapter extends BaseAdapter {
    private LayoutInflater mInflater;
    public FileListAdapter(Context context) {
      mInflater = LayoutInflater.from(context);
    }
    public int getCount() {
      if (mEntries==null) {
        return 0;
      }
      return mEntries.size();
    }
    public Object getItem(int position) {
      return position;
    }
    public long getItemId(int position) {
      return position;
    }
    public View getView(int position, View convertView, ViewGroup parent) {
      ViewHolder holder;
      if (convertView == null) {
        convertView = mInflater.inflate(android.R.layout.simple_list_item_1, null);
        holder = new ViewHolder();
        holder.text = (TextView) convertView.findViewById(android.R.id.text1);
        convertView.setTag(holder);
      }
      else {
        holder = (ViewHolder) convertView.getTag();
      }
      File f = mEntries.get(position);
      if (f.isDirectory()) {
        holder.text.setText("[DIR] "+f.getName());
      }
      else {
        holder.text.setText(f.getName());
      }
      return convertView;
    }
  }
  private class ViewHolder {
    TextView text;
  }
}

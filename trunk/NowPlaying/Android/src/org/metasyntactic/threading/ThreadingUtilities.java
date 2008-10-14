//Copyright 2008 Cyrus Najmabadi
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//limitations under the License.

package org.metasyntactic.threading;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import org.metasyntactic.ui.GlobalActivityIndicator;

/** @author cyrusn@google.com (Cyrus Najmabadi) */
public class ThreadingUtilities {
  public static boolean isMainThread() {
    return Looper.getMainLooper() == Looper.myLooper();
  }


  public static boolean isBackgroundThread() {
    return !isMainThread();
  }


  public static void performOnMainThread(Runnable runnable) {
    if (isMainThread()) {
      runnable.run();
    } else {
      new Handler(Looper.getMainLooper()).post(runnable);
    }
  }


  public static void performOnBackgroundThread(final Runnable runnable, final Object lock, final boolean visible) {
    final Object lock2 = lock == null ? new Object() : lock;

    Thread t = new HandlerThread("") {
      public void run() {
        synchronized (lock2) {
          try {
            GlobalActivityIndicator.addBackgroundTask(visible);
            runnable.run();
          } finally {
            GlobalActivityIndicator.removeBackgroundTask(visible);
          }
        }
      }
    };

    if (!visible) {
      t.setPriority(Thread.MIN_PRIORITY);
    }

    t.start();
  }
}

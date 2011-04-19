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

package com.tonchidot.o3d;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.HashSet;
import android.util.Log;

public final class Utils {
    private static HashSet<String> FEATURES = new HashSet<String>();
    static {
        // Detect features.
        // Interesting features are:
        // * 'neon', which means the CPU supports NEON vector instructions
        // * 'vfpv3', which means the CPU has a VFPv3
        // * 'half', which means hardware supports __fp16 natively
        try {
            BufferedReader buf = new BufferedReader(new FileReader("/proc/cpuinfo"));
            String line;
            while ((line=buf.readLine()) != null) {
                if (!line.startsWith("Features")) continue;
                String[] features = line.split(":")[1].trim().split(" ");
                for (String feat : features) {
                    Log.i("CPUINFO", String.format("Found feature '%s'", feat));
                    FEATURES.add(feat);
                }
                break;
            }
        }
        catch(IOException ex) {
            Log.e("CPUINFO", ex.getMessage());
        }
    }
    
    private Utils() {
        throw new AssertionError();
    }

    public static boolean hasFeature(String feature) {
        return FEATURES.contains(feature);
    }
}

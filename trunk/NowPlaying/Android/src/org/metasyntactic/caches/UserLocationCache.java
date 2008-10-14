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

package org.metasyntactic.caches;

import org.metasyntactic.Application;
import org.metasyntactic.data.Location;
import org.metasyntactic.threading.ThreadingUtilities;
import static org.metasyntactic.threading.ThreadingUtilities.performOnBackgroundThread;
import org.metasyntactic.utilities.*;
import org.w3c.dom.Element;

import java.io.*;
import java.net.URLEncoder;
import java.util.Locale;

public class UserLocationCache {
  private final Object lock = new Object();


  public UserLocationCache() {
  }


  public void downloadUserAddressLocation(final String userAddress) {
    Runnable runnable = new Runnable() {
      public void run() {
        downloadUserAddressLocationBackgroundEntryPoint(userAddress);
      }
    };

    performOnBackgroundThread(runnable, lock, true/*visible*/);
  }


  public Location downloadUserAddressLocationBackgroundEntryPoint(String userAddress) {
    assert ThreadingUtilities.isBackgroundThread();

    if (StringUtilities.isNullOrEmpty(userAddress)) {
      return null;
    }

    Location location = locationForUserAddress(userAddress);

    if (location == null) {
      location = downloadAddressLocationFromWebService(massageAddress(userAddress));
      if (!location.getCountry().equals(userCountryISO())) {
        location = downloadAddressLocationFromWebService(userAddress);
      }

      saveLocation(location, userAddress);
    }

    return location;
  }


  private static boolean containsNumber(String string) {
    for (int i = 0; i < string.length(); i++) {
      char c = string.charAt(i);
      if (c >= '0' && c <= '9') {
        return true;
      }
    }

    return false;
  }


  private static String userCountryISO() {
    return Locale.getDefault().getCountry();
  }


  private static String massageAddress(String userAddress) {
    userAddress = userAddress.trim();
    if (userAddress.length() <= 7 &&
        containsNumber(userAddress)) {
      // possibly a postal code.  append the country to help make it unique

      // we append the US name for the country here since that's what the
      // backend prefers
      String country = Locale.getDefault().getDisplayCountry(Locale.US);
      if (!StringUtilities.isNullOrEmpty(country)) {
        return userAddress + ". " + country;
      }
    }

    return null;
  }


  public Location locationForUserAddress(String userAddress) {
    if (StringUtilities.isNullOrEmpty(userAddress)) {
      return null;
    }

    Location location = loadLocation(massageAddress(userAddress));
    if (location != null) {
      return location;
    }

    return loadLocation(userAddress);
  }


  private static Location processResult(Element resultElement) {
    if (resultElement != null) {
      String latitude = resultElement.getAttribute("latitude");
      String longitude = resultElement.getAttribute("longitude");
      String address = resultElement.getAttribute("address");
      String city = resultElement.getAttribute("city");
      String state = resultElement.getAttribute("state");
      String country = resultElement.getAttribute("country");
      String postalCode = resultElement.getAttribute("zipcode");

      if (!StringUtilities.isNullOrEmpty(latitude) && !StringUtilities.isNullOrEmpty(longitude)) {
        return new Location(Double.parseDouble(latitude), Double.parseDouble(longitude), address, city, state,
            postalCode, country);
      }
    }

    return null;
  }


  private Location downloadAddressLocationFromWebService(String address) {
    if (StringUtilities.isNullOrEmpty(address)) {
      return null;
    }

    Location result = downloadAddressLocationFromWebServiceWorker(address);
    if (result != null && result.getLatitude() != 0 && result.getLongitude() != 0) {
      if (StringUtilities.isNullOrEmpty(result.getPostalCode())) {
        Location resultLocation = LocationUtilities.findLocation(result.getLatitude(), result.getLongitude());
        if (!StringUtilities.isNullOrEmpty(resultLocation.getPostalCode())) {
          return new Location(result.getLatitude(), result.getLongitude(), result.getAddress(), result.getCity(),
              result.getState(), resultLocation.getPostalCode(), result.getCountry());
        }
      }
    }

    return result;
  }


  private Location downloadAddressLocationFromWebServiceWorker(String address) {
    try {
      String escapedAddress = "http://metaboxoffice2.appspot.com/LookupLocation?q=" + URLEncoder.encode(address,
          "UTF-8");

      Element element = NetworkUtilities.downloadXml(escapedAddress, true);

      return processResult(element);
    } catch (UnsupportedEncodingException e) {
      throw new RuntimeException(e);
    }
  }


  private Location loadLocation(String address) {
    if (!StringUtilities.isNullOrEmpty(address)) {
      try {
        ObjectInputStream in = new ObjectInputStream(new FileInputStream(locationFile(address)));
        return (Location) in.readObject();
      } catch (IOException e) {
        ExceptionUtilities.log(UserLocationCache.class, "LoadLocation", e);
        return null;
      } catch (ClassNotFoundException e) {
        ExceptionUtilities.log(UserLocationCache.class, "LoadLocation", e);
        return null;
      }
    }

    return null;
  }


  private File locationFile(String address) {
    return new File(Application.userLocationsDirectory, FileUtilities.sanitizeFileName(address));
  }


  private void saveLocation(Location location, String address) {
    if (location == null || StringUtilities.isNullOrEmpty(address)) {
      return;
    }

    FileUtilities.writeObject(location, locationFile(address));
  }
}
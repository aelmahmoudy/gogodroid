/**
 *  This file is part of GogoDroid.
 *  http://code.google.com/p/gogodroid
 *
 *  GogoDroid is open source software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  GogoDroid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GogoDroid.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  @author أحمد المحمودي (Ahmed El-Mahmoudy) <aelmahmoudy@sabily.org>
 */
package com.googlecode.gogodroid;

interface GogoServiceIface {
    void startGogoc();
    void stopGogoc(boolean userStop);
    boolean statusGogoc();
    String statusConnection(boolean fromConnThread);
    String loadConf();
    void stateChanged();
}

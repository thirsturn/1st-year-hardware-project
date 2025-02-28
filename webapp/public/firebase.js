import { initializeApp } from "https://www.gstatic.com/firebasejs/9.21.0/firebase-app.js";
import { getDatabase, ref, query, limitToLast, onValue } from "https://www.gstatic.com/firebasejs/9.21.0/firebase-database.js";

// Firebase Configuration
const firebaseConfig = {
    apiKey: "AIzaSyCe-yzbhTygglWS-TeLGHBEpdsszigClpI",
    authDomain: "systeme-strasse.firebaseapp.com",
    databaseURL: "https://systeme-strasse-default-rtdb.asia-southeast1.firebasedatabase.app",
    projectId: "systeme-strasse",
    storageBucket: "systeme-strasse.appspot.com",
    messagingSenderId: "667582439789",
    appId: "1:667582439789:web:c313ad91bd89a9877f7a64"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
const database = getDatabase(app);

// Function to Display Sensor Data
function displaySensorData(data) {
    document.getElementById('humidity').textContent = data.humidity + ' %';
    document.getElementById('temperature').textContent = data.temperature + ' °C';
    document.getElementById('uv-int').textContent = data.uv + ' µW/cm²';
    document.getElementById('co2-lev').textContent = data.co2 + ' ppm';
    document.getElementById('dust').textContent = data.dust + ' µg/m³';
    document.getElementById('aqi').textContent = data.aqi;
}

// Query to Get the Latest Data
const sensorDataRef = query(ref(database, 'sensor_data'), limitToLast(1));

// Listen for Realtime Updates
onValue(sensorDataRef, (snapshot) => {
    snapshot.forEach((childSnapshot) => {
        const data = childSnapshot.val();
        if (data) {
            displaySensorData(data);
        } else {
            console.log('No data available');
        }
    });
}, (error) => {
    console.error('Error: ' + error.code);
});

// Realtime clock
function updateClock() {
    const now = new Date();
    const hours = String(now.getHours()).padStart(2, '0');
    const minutes = String(now.getMinutes()).padStart(2, '0');
    const seconds = String(now.getSeconds()).padStart(2, '0');
    const timeString = `${hours}:${minutes}:${seconds}`;
    document.getElementById('clock').textContent = timeString;
}

// Update the clock every second
setInterval(updateClock, 1000);

// Initial call to display the time immediately
updateClock();
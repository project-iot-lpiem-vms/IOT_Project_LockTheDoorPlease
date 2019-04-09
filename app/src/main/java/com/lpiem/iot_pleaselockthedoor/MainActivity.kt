package com.lpiem.iot_pleaselockthedoor

import android.os.Bundle
import android.util.Log
import android.view.View.GONE
import android.view.View.VISIBLE
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.isVisible
import com.google.firebase.FirebaseApp
import com.google.firebase.database.DataSnapshot
import com.google.firebase.database.DatabaseError
import com.google.firebase.database.FirebaseDatabase
import com.google.firebase.database.ValueEventListener
import kotlinx.android.synthetic.main.activity_main.*
import java.text.SimpleDateFormat
import java.util.*
const val ON = "on"
const val OFF = "off"
const val MESSAGE = "message"
const val RESULT = "result"
const val DATE_PATTERN = "dd/MM/yyyy"
const val HOUR_PATTERN = "hh 'h\n'mm'min\n' ss'sec\n'"

class MainActivity : AppCompatActivity() {

    private var isActivated = false
    private var intrusion = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        FirebaseApp.initializeApp(this)
        val database = FirebaseDatabase.getInstance()
        val myRef = database.getReference(MESSAGE)
        val stopAlarmRef = database.getReference(RESULT)

        btnStopAlarm.setOnClickListener {
            if (intrusion) {
                stopAlarmRef.setValue(OFF)
                myRef.setValue(getString(R.string.alarmOFF))
                btnStopAlarm.text = getString(R.string.activateAlarm)
                isActivated = false
                intrusion = false
                return@setOnClickListener
            }
            isActivated = if (!isActivated) {
                stopAlarmRef.setValue(ON)
                myRef.setValue(getString(R.string.alarmON))
                btnStopAlarm.text = getString(R.string.deactivateAlarm)
                true
            } else {
                stopAlarmRef.setValue(OFF)
                myRef.setValue(getString(R.string.alarmOFF))
                btnStopAlarm.text = getString(R.string.activateAlarm)
                false
            }
        }

        stopAlarmRef.addListenerForSingleValueEvent(object : ValueEventListener{

            override fun onCancelled(error: DatabaseError) {
                Log.w("TAG", "Failed to read value.", error.toException())
            }

            override fun onDataChange(snapShot: DataSnapshot) {
                val value = snapShot.getValue(String::class.java)
                if (value == ON && alertTitle.isVisible) {
                    myRef.setValue(getString(R.string.alarmOFF))
                    stopAlarmRef.setValue(OFF)
                    alertTitle.visibility = GONE
                } else if (value == ON) {
                    alertTitle.visibility = GONE
                } else if (value == OFF) {
                    alertTitle.visibility = GONE
                }
            }

        })

        myRef.addValueEventListener(object : ValueEventListener {

            override fun onDataChange(dataSnapshot: DataSnapshot) {
                val value = dataSnapshot.getValue(String::class.java)
                Log.d("TAG", "Value is: " + value!!)
                test.text = value
                when (value) {
                    getString(R.string.alarmOFF) -> {
                        stopAlarmRef.setValue(OFF)
                        btnStopAlarm.text = getString(R.string.activateAlarm)
                        alertTitle.visibility = GONE
                    }
                    getString(R.string.alarmON) -> {
                        stopAlarmRef.setValue(ON)
                        btnStopAlarm.text = getString(R.string.deactivateAlarm)
                        alertTitle.visibility = GONE
                    }
                    else -> {
                        alertTitle.visibility = VISIBLE
                        val hourFormatter = SimpleDateFormat(HOUR_PATTERN)
                        val dateFormatter = SimpleDateFormat(DATE_PATTERN)
                        val date = dateFormatter.format(Date(System.currentTimeMillis()))
                        val hour = hourFormatter.format(Date(System.currentTimeMillis()))
                        test.text = String.format(getString(R.string.intrusionMessage), hour, date)
                        intrusion = true
                    }
                }
            }

            override fun onCancelled(error: DatabaseError) {
                // Failed to read value
                Log.w("TAG", "Failed to read value.", error.toException())
            }
        })

    }

}

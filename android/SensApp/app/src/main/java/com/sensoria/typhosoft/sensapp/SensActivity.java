package com.sensoria.typhosoft.sensapp;

import android.content.Intent;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AppCompatActivity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;

import com.sensoria.typhosoft.sensapp.custom.adapter.SensAdapter;
import com.sensoria.typhosoft.sensapp.datamodel.SensModel;
import com.sensoria.typhosoft.sensapp.network.SensClient;
import com.sensoria.typhosoft.sensapp.service.SensAppService;

public class SensActivity extends AppCompatActivity {
    private SensAdapter adapter;
    private SensClient sensClient;
    private EditText cmd;
    private ListView listView;
    private ListView navListView;
    private ArrayAdapter<String> menuAdapter;
    private ActionBarDrawerToggle drawerToggle;
    private DrawerLayout drawerLayout;
    private String activityTitle;
    private Intent serviceIntent;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        setupDrawer();

        navListView = (ListView) findViewById(R.id.navList);
        initMenu();
        cmd = (EditText) findViewById(R.id.editTextCMD);

        sensClient = new SensClient(this);
        adapter = new SensAdapter(this, R.id.list, SensModel.getInstance().getItems(), sensClient);

        listView = (ListView) findViewById(R.id.list);
        listView.setAdapter(adapter);

        // serviceIntent = new Intent(this, SensAppService.class);
        // serviceIntent.setData(Uri.parse("sens://sensapp"));
        // startService(serviceIntent);

        sensClient.start();
    }

    private void setupDrawer() {
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setHomeButtonEnabled(true);
        drawerLayout = (DrawerLayout) findViewById(R.id.drawer_layout);
        activityTitle = getTitle().toString();

        drawerToggle = new ActionBarDrawerToggle(this, drawerLayout, R.string.drawer_open, R.string.drawer_close) {

            /** Called when a drawer has settled in a completely open state. */
            public void onDrawerOpened(View drawerView) {
                super.onDrawerOpened(drawerView);
                getSupportActionBar().setTitle("Navigation!");
                invalidateOptionsMenu(); // creates call to onPrepareOptionsMenu()
            }

            /** Called when a drawer has settled in a completely closed state. */
            public void onDrawerClosed(View view) {
                super.onDrawerClosed(view);
                getSupportActionBar().setTitle(activityTitle);
                invalidateOptionsMenu(); // creates call to onPrepareOptionsMenu()
            }
        };

        drawerToggle.setDrawerIndicatorEnabled(true);
        drawerLayout.setDrawerListener(drawerToggle);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        // Activate the navigation drawer toggle
        if (drawerToggle.onOptionsItemSelected(item)) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        drawerToggle.syncState();

        sensClient.sendMessage(getResources().getString(R.string.command));
    }



    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        drawerToggle.onConfigurationChanged(newConfig);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds sensItems to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    private void initMenu() {
        String[] osArray = {"Network", "Automation", "Settings"};
        menuAdapter = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, osArray);
        navListView.setAdapter(menuAdapter);
        navListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                // open activity
            }
        });
    }

    public void sendUdpPacket(View view) {
        sensClient.sendMessage(cmd.getText().toString() + "\n");
    }

    public void updateSens() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                adapter.notifyDataSetChanged();
            }
        });
    }
}

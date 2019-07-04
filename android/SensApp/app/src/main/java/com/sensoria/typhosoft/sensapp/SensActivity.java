package com.sensoria.typhosoft.sensapp;

import android.content.res.Configuration;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AppCompatActivity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ListView;

import com.sensoria.typhosoft.sensapp.core.SensAdapter;
import com.sensoria.typhosoft.sensapp.network.SensClient;
import com.sensoria.typhosoft.sensapp.network.SensDiscovery;
import com.sensoria.typhosoft.sensapp.network.SesSocketSingleton;

public class SensActivity extends AppCompatActivity {
    private SensAdapter adapter;
    private SensClient sensClient;
    private EditText cmd;
    private CheckBox autoDiscovery;
    private ListView listView;
    private ListView navListView;
    private ArrayAdapter<String> menuAdapter;
    private ActionBarDrawerToggle drawerToggle;
    private DrawerLayout drawerLayout;
    private String activityTitle;
    private SensDiscovery discovery;
    //private Intent serviceIntent;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        setupDrawer();

        navListView = (ListView) findViewById(R.id.navList);
        initMenu();
        cmd = (EditText) findViewById(R.id.editTextCMD);
        autoDiscovery = (CheckBox) findViewById(R.id.autoDiscover);
        autoDiscovery.setChecked(true);
        autoDiscovery.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    discovery.pause();
                } else {
                    discovery.pause();
                }
            }
        });
        sensClient = new SensClient(this);
        sensClient.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        adapter = new SensAdapter(this, R.id.list, sensClient);

        listView = (ListView) findViewById(R.id.list);
        listView.setAdapter(adapter);


        // use this to start and trigger a service
        //    serviceIntent = new Intent(this, SensAppService.class);
        // potentially add data to the intent
        //    serviceIntent.putExtra("KEY1", "Value to be used by the service");
        //    startService(serviceIntent);


        discovery = new SensDiscovery(autoDiscovery);
        discovery.start();
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
        SesSocketSingleton.getInstance().sendMessage(cmd.getText().toString() + "\n");
    }

    public void updateSens() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                adapter.notifyDataSetChanged();
            }
        });
    }


    @Override
    protected void onStart() {
        super.onStart();
        resume();
    }

    private void resume() {
        if (sensClient == null) {
            sensClient = new SensClient(this);
            sensClient.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        }

        discovery.OnResume();
    }

    @Override
    protected void onStop() {
        super.onStop();
        discovery.OnPause();
        sensClient.exit();
        sensClient = null;
    }


    @Override
    protected void onPause() {
        super.onPause();
        discovery.OnPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        resume();
    }

}

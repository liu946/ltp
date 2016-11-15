package cn.edu.hit.ir.ltpmobel;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import java.io.File;
import java.util.Timer;
import java.util.TimerTask;

public class MainActivity extends AppCompatActivity {

    private EditText textView;
    private EditText inputTextView;

    protected void output(String msg) {
        textView.setText(msg);
    }


    protected String ParserTestDefaultXmlString;
    private void SetDefaultString() {
        ParserTestDefaultXmlString = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
                + "<xml4nlp>"
                + "    <note sent=\"y\" word=\"y\" pos=\"y\" ne=\"y\" parser=\"n\" wsd=\"n\" srl=\"n\" />"
                + "    <doc>"
                + "        <para id=\"0\">"
                + "            <sent id=\"0\" cont=\"北京欢迎你！\">"
                + "                <word id=\"0\" cont=\"北京\" pos=\"ns\" ne=\"S-Ns\" />"
                + "                <word id=\"1\" cont=\"欢迎\" pos=\"v\" ne=\"O\" />"
                + "                <word id=\"2\" cont=\"你\" pos=\"r\" ne=\"O\" />"
                + "                <word id=\"3\" cont=\"！\" pos=\"wp\" ne=\"O\" />"
                + "            </sent>"
                + "        </para>"
                + "    </doc>"
                + "</xml4nlp>";
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        SetDefaultString();

        textView = (EditText)findViewById(R.id.textView);
        inputTextView = (EditText)findViewById(R.id.tf_sentence);

        Button initBtn = (Button)findViewById(R.id.btn_initLTP);
        initBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String path = Environment.getExternalStorageDirectory().getAbsolutePath();
                initLTP(path + "/Download/");
            }
        });

        Button analizeBtn = (Button)findViewById(R.id.btn_predict);
        analizeBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String sentence = String.valueOf(inputTextView.getText());
                if (sentence.equals("")) {
                    sentence = "我爱北京天安门。";
                }
                String res = LTPAnalize(sentence);
                Log.v("info", res);
                textView.setText(res);
            }
        });

        Button freeBtn = (Button)findViewById(R.id.btn_free);
        freeBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                freeLTP();
            }
        });

        Button initLTPNDPCKBtn = (Button)findViewById(R.id.btn_initLTPNDPCK);
        initLTPNDPCKBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String path = Environment.getExternalStorageDirectory().getAbsolutePath();
                initLTPNDPCK(path + "/Download/");
            }
        });

        Button LTPNDPCK_loadParserBtn = (Button)findViewById(R.id.btn_loadParser);
        LTPNDPCK_loadParserBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                LTPNDPCKloadParser();
            }
        });

        Button testParserBtn = (Button)findViewById(R.id.btn_testParser);
        testParserBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String xml = String.valueOf(inputTextView.getText());
                if (xml.equals("")) {
                    xml =  ParserTestDefaultXmlString;
                }
                String res = LTPNDPCKParser(xml);
                textView.setText(res);
            }
        });

        // 多次测试Parser速度
        Button testParserNBtn = (Button)findViewById(R.id.btn_testParserN);
        testParserNBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String xml = String.valueOf(inputTextView.getText());
                if (xml.equals("")) {
                    xml =  ParserTestDefaultXmlString;
                }
                long startMili = System.currentTimeMillis();
                int runtimes = 1000;
                for (int i = 0; i < runtimes; i++) {
                    LTPNDPCKParser(xml);
                }
                long endMili = System.currentTimeMillis();

                textView.setText(
                        String.format("Main Process Run" +
                                        "Times: %d\n" +
                                        "Time: %d ms\n" +
                                        "Processer: 1\n" +
                                        "SentenceXml: \n%s\n",
                                runtimes ,endMili - startMili, xml));
            }
        });
        // 多次测试Parser速度，多线程
        Button testParserNMTPcsBtn = (Button)findViewById(R.id.btn_testParserMTPcs);


        // 多次测试LTP速度
        Button testLTPNBTN = (Button)findViewById(R.id.btn_testLTPN);
        testLTPNBTN.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String sentence = String.valueOf(inputTextView.getText());
                if (sentence.equals("")) {
                    sentence = "我爱北京天安门。";
                }
                long startMili = System.currentTimeMillis();
                int runtimes = 100;
                for (int i = 0; i < runtimes; i++) {
                    LTPAnalize(sentence);
                }
                long endMili = System.currentTimeMillis();

                textView.setText(
                        String.format("Main Process Run" +
                                        "Times: %d\n" +
                                        "Time: %d ms\n" +
                                        "Processer: 1\n" +
                                        "Sentence: \n%s\n",
                                runtimes ,endMili - startMili, sentence));
            }
        });

        // 多次测试LTP速度，多线程
        Button testLTPNMTPcsBTN = (Button)findViewById(R.id.btn_testLTPNMTPcs);



    }

    // native C++ code for LTP
    public native boolean initLTP(String path);
    public native String LTPAnalize(String sentence);
    public native boolean freeLTP();

    public native boolean initLTPNDPCK(String path);
    public native boolean LTPNDPCKloadParser();
    public native String LTPNDPCKParser(String sentence);




    /** Load jni .so on initialization */
    static {
        System.loadLibrary("ltp");
    }
}

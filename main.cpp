#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"

#define LED_PIN  16
static const struct device *led_dev;
static const struct device *imu_dev;
static bool led_ok = false;
static bool imu_ok = false;
static float ei_features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

static const char *keywords[] = {
    "turn","on","off","led","light","switch","enable","disable",
    "activate","deactivate","blink","flash","toggle","strobe","flicker",
    "temp","temperature","hot","cold","warm","heat","degrees","thermal",
    "status","check","system","state","report","happening","going",
    "hello","hi","hey","good","morning","evening","greet","howdy","welcome","hola",
    "please","want","make","show","give","tell","read","get","can","start",
    "power","put","kill","shut","stop","set","do","up","bright","dark","mode",
    "night","alive","working","help","joke","funny","laugh","time","what",
    "party","disco","celebrate","rave","dance","sleep","quiet","dim","max",
    "full","low","yo","sup","wassup","hiya","heya","afternoon",
    "okay","ok","fine","great","awesome","cool","nice","bad","wrong","broken",
    "you","are","am","is","feel","feeling","weather","outside","inside","room",
    "burning","freezing","boiling","chilly","humid","dry","fire","ice","snow",
    "who","name","call","remember","forget","know","think","believe","sure",
    "thanks","thank","bye","goodbye","later","cya","see","rest",
    "reboot","restart","reset","refresh","update","info","version","build",
    "imu","axis","accel","gyro","shake","motion","tilt","moving","accelerometer","gyroscope"
};
#define NUM_KEYWORDS (sizeof(keywords)/sizeof(keywords[0]))

/* ── LED helpers ── */
static void led_set(int val) {
    if (led_ok) gpio_pin_set(led_dev, LED_PIN, val);
}
static void led_blink(int times, int delay_ms) {
    for (int i = 0; i < times*2; i++) {
        if (led_ok) gpio_pin_toggle(led_dev, LED_PIN);
        k_msleep(delay_ms);
    }
    led_set(0);
}

/* ── IMU read ── */
static void read_imu(void) {
    if (!imu_ok) {
        printk("[ZephyrAI] IMU not found! Check SDA=GPIO5 SCL=GPIO4\n");
        return;
    }

    struct sensor_value ax, ay, az, gx, gy, gz;
    sensor_sample_fetch(imu_dev);
    sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_X, &ax);
    sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_Y, &ay);
    sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_Z, &az);
    sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_X, &gx);
    sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_Y, &gy);
    sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_Z, &gz);

    double axd = sensor_value_to_double(&ax);
    double ayd = sensor_value_to_double(&ay);
    double azd = sensor_value_to_double(&az);
    double gxd = sensor_value_to_double(&gx);
    double gyd = sensor_value_to_double(&gy);
    double gzd = sensor_value_to_double(&gz);

    double mag = sqrt(axd*axd + ayd*ayd + azd*azd);

    printk("[ZephyrAI] ======= LSM6DSOX IMU DATA =======\n");
    printk("[ZephyrAI]  Accel X : %6.3f m/s2\n", axd);
    printk("[ZephyrAI]  Accel Y : %6.3f m/s2\n", ayd);
    printk("[ZephyrAI]  Accel Z : %6.3f m/s2\n", azd);
    printk("[ZephyrAI]  Magnitude: %6.3f m/s2\n", mag);
    printk("[ZephyrAI] -----------------------------------\n");
    printk("[ZephyrAI]  Gyro  X : %6.3f rad/s\n", gxd);
    printk("[ZephyrAI]  Gyro  Y : %6.3f rad/s\n", gyd);
    printk("[ZephyrAI]  Gyro  Z : %6.3f rad/s\n", gzd);
    printk("[ZephyrAI] ===================================\n");

    /* Shake detection */
    if (mag > 15.0) {
        printk("[ZephyrAI] WHOA! Detected a shake! Magnitude: %.2f m/s2\n", mag);
        led_blink(4, 80);
    } else if (mag > 12.0) {
        printk("[ZephyrAI] Slight movement detected (%.2f m/s2)\n", mag);
    } else {
        printk("[ZephyrAI] Board is nice and still (%.2f m/s2)\n", mag);
    }

    /* Tilt detection */
    if (azd < 5.0 && azd > -5.0) {
        printk("[ZephyrAI] Tilt: Board is on its side!\n");
    } else if (azd < 0) {
        printk("[ZephyrAI] Tilt: Board is upside down! How rude.\n");
    } else {
        printk("[ZephyrAI] Tilt: Board is flat. Very professional.\n");
    }
}

/* ── Feature extraction ── */
static void text_to_features(const char *input) {
    memset(ei_features, 0, sizeof(ei_features));
    char buf[128];
    strncpy(buf, input, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';
    for (int i = 0; buf[i]; i++)
        if (buf[i] >= 'A' && buf[i] <= 'Z') buf[i] += 32;
    char *tok = strtok(buf, " \t\r\n");
    while (tok) {
        for (int i = 0; i < (int)NUM_KEYWORDS && i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i++)
            if (strcmp(tok, keywords[i]) == 0) ei_features[i] = 1.0f;
        tok = strtok(NULL, " \t\r\n");
    }
}

/* ── Quick IMU keyword check (before classifier) ── */
static bool is_imu_query(const char *input) {
    const char *imu_words[] = {"imu","axis","accel","gyro","shake","motion",
                                "tilt","moving","accelerometer","gyroscope"};
    char buf[128];
    strncpy(buf, input, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';
    for (int i = 0; buf[i]; i++)
        if (buf[i] >= 'A' && buf[i] <= 'Z') buf[i] += 32;
    for (int j = 0; j < 10; j++) {
        if (strstr(buf, imu_words[j])) return true;
    }
    return false;
}

/* ── Response banks ── */
static const char *greetings[] = {
    "[ZephyrAI] Hey hey hey! ZephyrAI at your service, human!\n",
    "[ZephyrAI] Oh! A visitor! I was just counting clock cycles. What's up?\n",
    "[ZephyrAI] Good morning! Or evening. I don't have a clock. Please send help.\n",
    "[ZephyrAI] Hello there! Running on 240MHz and pure optimism!\n",
    "[ZephyrAI] Yo! ZephyrAI here. Embedded, enlightened, and slightly bored.\n",
    "[ZephyrAI] Greetings, carbon-based life form! How can I serve you today?\n",
    "[ZephyrAI] Hi! I would offer you a coffee but I only have GPIO pins.\n",
    "[ZephyrAI] Hey! I was just thinking about null pointers. Glad you are here.\n",
};
#define NUM_GREETINGS (sizeof(greetings)/sizeof(greetings[0]))

static const char *unknown_msgs[] = {
    "[ZephyrAI] I understood approximately 0%% of that. Try again?\n",
    "[ZephyrAI] Hmm. My training data did not cover that. Try 'help'!\n",
    "[ZephyrAI] Error 404: Intent not found. Try turning me off and on?\n",
    "[ZephyrAI] That went straight over my 16KB instruction cache.\n",
    "[ZephyrAI] I speak LED and temperature. That was neither. Impressive.\n",
    "[ZephyrAI] Low confidence detected. Are you speaking human? Try again!\n",
    "[ZephyrAI] My neural network is confused. And honestly, same.\n",
};
#define NUM_UNKNOWN (sizeof(unknown_msgs)/sizeof(unknown_msgs[0]))

static uint32_t response_counter = 0;

/* ── Inference ── */
static void run_inference(const char *input) {

    /* IMU fast-path — no classifier needed */
    if (is_imu_query(input)) {
        read_imu();
        return;
    }

    text_to_features(input);
    signal_t signal;
    ei_impulse_result_t result;
    if (numpy::signal_from_buffer(ei_features,
            EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal) != 0) {
        printk("[ZephyrAI] Signal error\n"); return;
    }
    if (run_classifier(&signal, &result, false) != EI_IMPULSE_OK) {
        printk("[ZephyrAI] Classifier error\n"); return;
    }

    float best_score = 0.0f;
    const char *best_label = "UNKNOWN";
    for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        printk("  %-12s %.2f\n",
               result.classification[i].label,
               (double)result.classification[i].value);
        if (result.classification[i].value > best_score) {
            best_score = result.classification[i].value;
            best_label = result.classification[i].label;
        }
    }
    printk("[ZephyrAI] >> %s (%.0f%%)\n", best_label, (double)(best_score*100));
    response_counter++;

    if (best_score < 0.70f) {
        printk("%s", unknown_msgs[response_counter % NUM_UNKNOWN]);
        return;
    }

    if (strcmp(best_label, "LED_ON") == 0) {
        led_set(1);
        printk(led_ok ?
            "[ZephyrAI] Let there be light! LED is ON!\n" :
            "[ZephyrAI] LED ON! (Spiritually at least - GPIO not found)\n");

    } else if (strcmp(best_label, "LED_OFF") == 0) {
        led_set(0);
        printk(led_ok ?
            "[ZephyrAI] Lights out! Sweet darkness.\n" :
            "[ZephyrAI] LED OFF! (Already dark in here anyway)\n");

    } else if (strcmp(best_label, "BLINK") == 0) {
        printk("[ZephyrAI] PARTY MODE ACTIVATED! Hold on to your electrons!\n");
        led_blink(8, 150);
        printk("[ZephyrAI] Okay that was fun. Back to being professional.\n");

    } else if (strcmp(best_label, "TEMP") == 0) {
        float t = 28.0f + (k_uptime_get() % 5000) / 1000.0f;
        if (t > 31.0f)
            printk("[ZephyrAI] Temperature: %.1f C - It is getting spicy in here!\n", (double)t);
        else if (t < 29.0f)
            printk("[ZephyrAI] Temperature: %.1f C - Nice and chill, like my code.\n", (double)t);
        else
            printk("[ZephyrAI] Temperature: %.1f C - Perfectly room temperature, like a good API.\n", (double)t);

    } else if (strcmp(best_label, "STATUS") == 0) {
        int s = led_ok ? gpio_pin_get(led_dev, LED_PIN) : 0;
        uint32_t uptime = (uint32_t)(k_uptime_get() / 1000);
        printk("[ZephyrAI] ==================================\n");
        printk("[ZephyrAI]        SYSTEM STATUS REPORT\n");
        printk("[ZephyrAI] ==================================\n");
        printk("[ZephyrAI]  LED      : %s\n", s ? "ON" : "OFF");
        printk("[ZephyrAI]  IMU      : %s\n", imu_ok ? "LSM6DSOX OK" : "Not found");
        printk("[ZephyrAI]  Uptime   : %u seconds\n", uptime);
        printk("[ZephyrAI]  Classes  : %d intents loaded\n", EI_CLASSIFIER_LABEL_COUNT);
        printk("[ZephyrAI]  Features : %d keywords\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
        printk("[ZephyrAI]  Requests : %u handled\n", response_counter);
        printk("[ZephyrAI]  Mood     : %s\n", uptime < 60 ? "Fresh and excited!" : "Experienced and wise.");
        printk("[ZephyrAI] ==================================\n");

    } else if (strcmp(best_label, "GREETING") == 0) {
        printk("%s", greetings[response_counter % NUM_GREETINGS]);
        led_blink(2, 100);
        printk("[ZephyrAI] (Try: imu | shake | axis | gyro | tilt)\n");
    }
}

/* ── UART input ── */
static const struct device *uart_dev;

static int read_line(char *buf, int maxlen) {
    int i = 0;
    unsigned char c;
    while (i < maxlen - 1) {
        if (uart_dev) {
            while (uart_poll_in(uart_dev, &c) < 0) k_msleep(1);
        } else {
            int ch;
            do { ch = getchar(); } while (ch < 0);
            c = (unsigned char)ch;
        }
        if (c == '\r' || c == '\n') {
            buf[i] = '\0';
            printk("\n");
            return i;
        }
        buf[i++] = (char)c;
        if (uart_dev) uart_poll_out(uart_dev, c);
        else printk("%c", c);
    }
    buf[i] = '\0';
    return i;
}

/* ── Main ── */
int main(void) {
    uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart0));
    if (!device_is_ready(uart_dev)) uart_dev = NULL;

    printk("\n");
    printk("╔══════════════════════════════════════╗\n");
    printk("║      ZephyrAI Assistant v2.1         ║\n");
    printk("║  ESP32-S3 + TinyML + LSM6DSOX IMU   ║\n");
    printk("╚══════════════════════════════════════╝\n\n");

    /* LED init */
    led_dev = DEVICE_DT_GET(DT_NODELABEL(gpio1));
    if (device_is_ready(led_dev)) {
        gpio_pin_configure(led_dev, LED_PIN, GPIO_OUTPUT_INACTIVE);
        led_ok = true;
        printk("[ZephyrAI] LED ready! Doing the startup dance...\n");
        led_blink(4, 120);
    } else {
        printk("[ZephyrAI] LED not found - inference only mode\n");
    }

    /* IMU init */
    imu_dev = DEVICE_DT_GET(DT_NODELABEL(lsm6dsox));
    if (device_is_ready(imu_dev)) {
        imu_ok = true;
        printk("[ZephyrAI] LSM6DSOX IMU ready on I2C! (SDA=5, SCL=4)\n");
    } else {
        printk("[ZephyrAI] LSM6DSOX not found - check wiring SDA=5 SCL=4\n");
    }

    printk("[ZephyrAI] %d intents | %d features | Ready!\n\n",
           EI_CLASSIFIER_LABEL_COUNT, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    printk("  Commands: turn on led | turn off led | blink\n");
    printk("            temperature | show status  | good morning\n");
    printk("            imu | axis  | gyro | shake | tilt\n\n");

    char input[128];
    while (1) {
        printk("You: ");
        int len = read_line(input, sizeof(input));
        if (len == 0) continue;
        printk("[INPUT] %s\n", input);
        run_inference(input);
        printk("\n");
    }
    return 0;
}

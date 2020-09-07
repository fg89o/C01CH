<template>
<v-container style="max-width: 100%">
    <v-row dark style="background-color: rgb(51,51,51); margin-top: -12px; padding-bottom: 60px">
    <v-col cols="12"
    >
      <h3 dark style="color: white">FECHA Y HORA</h3>
    </v-col>
  </v-row>
    <v-row align="center" justify="center">
        <v-col cols="12" sm="10" lg="8"
        style="
            padding-top: 0px;
            margin-top: -50px;
            background-color: white;
            border-radius: 5px;
            border: 1px solid transparent;
            border-image: linear-gradient(to bottom, rgb(51,51,51) 0%, rgba(255,255,255,0) 50%);
            border-image-slice: 1;
        "
        >
            <p class="my-4 text-uppercase font-weight-light overline">Configuración > Fecha/Hora</p>
            <v-divider></v-divider>
            <h4 class="my-2">Opciones relacionadas con la configuración de fecha y hora</h4>
            <v-divider></v-divider>
            <v-alert
                outlined
                type="success"
                text
                transition="scale-transition"
                :value="success"
                dismissible
                v-model="success"
            >
            Cambios realizados correctamente!
            </v-alert>
            <v-alert
                outlined
                type="error"
                text
                transition="scale-transition"
                :value="error"
                dismissible
                v-model="error"
            >
            Error al guardar los datos. No se recibió respuesta.
            </v-alert>
            <v-row>
                <v-col cols="12">
                    <v-text-field
                        readonly
                        v-model="datenow"
                        label="Fecha y hora actual"
                        required
                        filled
                    ></v-text-field>
                </v-col>
            </v-row>
            <v-row v-bind:class="{ 'd-none': !ntpEnabled }">
                <v-col cols="12">
                    <v-text-field
                        v-model="ntpServername"
                        :counter="30"
                        label="Servidor NTP"
                        hint = "Por defecto: pool.ntp.org"
                        persistent-hint
                        required
                    ></v-text-field>
                </v-col>
                <v-col cols="12">
                    <v-autocomplete
                        v-model="timezone"
                        hint="Zona horaria para controlar la hora y el horario de verano/invierno"
                        :items="selectOptions"
                        :search-input.sync="search"
                        color="white"
                        hide-no-data
                        label="Zonas horarias"
                        placeholder="empiece a escribir para buscar"
                        return-object
                    ></v-autocomplete>
                </v-col>
            </v-row>
            <v-row v-bind:class="{ 'd-none': ntpEnabled }">
                <v-col cols="12">
                    <v-menu
                    v-model="menu2"
                    :close-on-content-click="false"
                    :nudge-right="40"
                    transition="scale-transition"
                    offset-y
                    min-width="290px"
                    >
                        <template v-slot:activator="{ on }">
                            <v-text-field
                            v-model="newDate"
                            label="Nueva fecha"
                            readonly
                            v-on="on"
                            ></v-text-field>
                        </template>
                        <v-date-picker v-model="newDate" @input="menu2 = false"></v-date-picker>
                    </v-menu>
                </v-col>
                <v-col cols="12">
                    <v-dialog
                    ref="dialog"
                    v-model="modal2"
                    :return-value.sync="time"
                    persistent
                    width="290px"
                    >
                    <template v-slot:activator="{ on }">
                        <v-text-field
                        v-model="time"
                        label="Nueva Hora"
                        readonly
                        v-on="on"
                        ></v-text-field>
                    </template>
                    <v-time-picker
                        v-if="modal2"
                        v-model="time"
                        full-width
                    >
                        <v-spacer></v-spacer>
                        <v-btn text color="primary" @click="modal2 = false">Cancel</v-btn>
                        <v-btn text color="primary" @click="$refs.dialog.save(time)">OK</v-btn>
                    </v-time-picker>
                    </v-dialog>
                </v-col>
            </v-row>
            <v-row>
                <v-col>
                    <v-divider class="py-2"></v-divider>
                    <v-spacer></v-spacer>
                    <v-btn large color="primary" v-on:click="save()">Guardar</v-btn>
                </v-col>
            </v-row>
        </v-col>
    </v-row>
</v-container>
</template>

<script>

import Zones from "../plugins/zones";

export default {
  name: 'Config_FechaHora',
  data: () => ({
        datenow: '1970-01-01',
        rtcDate: null,
        newDate:  new Date().toISOString().substr(0, 10),
        modal: false,
        modal2: false,
        menu2: false,
        time: '', //new Date().getHours().toString().padStart(2,"0") + ":"+new Date().getMinutes().toString().padStart(2,"0"),
        timeoutTime: null,
        ntpEnabled: true,
        timezone: "",
        success: false,
        error: false,
        ntpServername: '',
        selectOptions: [],
        search: ""
    }),
    methods: {
        updateTime() {

            var self = this;
            clearTimeout(this.timeoutTime);

            if (this.rtcDate != null)
            {
                this.rtcDate.setSeconds(this.rtcDate.getSeconds() + 1);

                var day = this.rtcDate.getUTCDate().toString().padStart(2,"0");
                var month = (this.rtcDate.getUTCMonth()+1).toString().padStart(2,"0");
                var year = this.rtcDate.getUTCFullYear().toString();
                var hour = this.rtcDate.getUTCHours().toString().padStart(2,"0");
                var minute = this.rtcDate.getUTCMinutes().toString().padStart(2,"0");

                this.datenow = day + "/" + month + "/" + year + " " + hour + ":" + minute;
            }

            this.timeoutTime = setTimeout(self.updateTime, 1000)
        },
        save()
        {
            var obj = {}
            if (this.ntpEnabled)
            {
                obj = {
                    enabled: true,
                    servername: this.ntpServername,
                    timezone: this.timezone,
                    timezonePosix: Zones[this.timezone]
                };
            }
            else
            {
                var hour = this.time.split(":")[0];
                var minute = this.time.split(":")[1];
                var mes = this.newDate.split("-")[1] - 1;

                var unixtime = Date.UTC(this.newDate.split("-")[0], mes, this.newDate.split("-")[2], hour, minute,0,0);
                
                obj = {
                    enabled: false,
                    unixtime: unixtime / 1000,
                    timezone: this.timezone,
                    timezonePosix: Zones[this.timezone]
                }
            }
            
            var self = this;
            this.$http.post(process.env.VUE_APP_REMOTESERVER + 'rtc', JSON.stringify(obj), { headers: {"Content-Type": "text/plain"}}
            ).then(function(/* response */){ 
                console.log("Fecha cambiada correctamente!");
                self.requestInfo();
                self.success = true;
            }, function(){
                self.requestInfo();
                self.error = true;
            });
        },
        requestInfo()
        {
            var self = this;

            self.error = false;
            self.success = false;

            this.$http.get(process.env.VUE_APP_REMOTESERVER + 'rtc').then(function(response){
                self.ntpEnabled = response.body["enabled"];
                self.timezone = response.body["timezone"];
                self.ntpServername = response.body["servername"];
                self.rtcDate = new Date(parseInt(response.body["unixtime"]) * 1000);
                self.updateTime();
            }, function(){
                self.error = true;
            });
        },
    },
    created: function(){
        
        var json = Zones;

        // Obteniendo todas las claves del JSON
        for (var clave in json){
            // Controlando que json realmente tenga esa propiedad
            if (Object.prototype.hasOwnProperty.call(json, clave)) {
                // Mostrando en pantalla la clave junto a su valor
                this.selectOptions.push(clave);
            }
        }

        this.requestInfo();
    }
}
</script>

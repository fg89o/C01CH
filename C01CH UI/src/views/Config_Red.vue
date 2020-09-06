<template>
<v-container style="max-width: 100%">
    <v-row dark style="background-color: rgb(51,51,51); margin-top: -12px; padding-bottom: 60px">
    <v-col cols="12"
    >
      <h3 dark style="color: white">RED</h3>
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
            <p class="my-4 px-4 text-uppercase font-weight-light overline">Configuración > Red</p>
            <v-divider></v-divider>
            <h4 class="my-2 px-4">Configuración actual de red</h4>
            <v-divider></v-divider>
            <v-row>
                <v-col cols="12">
                    <v-text-field
                        readonly
                        v-model="current_ssid"
                        label="SSID actual:"
                        required
                        dense
                    ></v-text-field>
                </v-col>
                <v-col cols="4">
                    <v-text-field
                        readonly
                        v-model="mode"
                        label="Modo"
                        required
                        dense
                    ></v-text-field>
                </v-col>
                <v-col cols="4">
                    <v-text-field
                        readonly
                        v-model="current_canal"
                        label="Canal"
                        required
                        dense
                    ></v-text-field>
                </v-col>
                <v-col cols="4">
                    <v-text-field
                        readonly
                        v-model="current_potencia"
                        label="Potencia"
                        required
                        dense
                    ></v-text-field>
                </v-col>
                <v-col cols="12" lg="4">
                    <v-text-field
                        readonly
                        v-model="current_ip"
                        label="IP local:"
                        required
                        dense
                    ></v-text-field>
                </v-col>
                <v-col cols="12" lg="4">
                    <v-text-field
                        readonly
                        v-model="current_gateway"
                        label="Puerta de enlace:"
                        required
                        dense
                    ></v-text-field>
                </v-col>
                <v-col cols="12" lg="4">
                    <v-text-field
                        readonly
                        v-model="current_dns"
                        label="DNS:"
                        required
                        dense
                    ></v-text-field>
                </v-col>
            </v-row>
            <v-row>
                <v-col cols="12">
                    <v-divider></v-divider>
                    <h4 class="my-2 px-4">Nueva configuración de red</h4>
                    <v-divider></v-divider>
                </v-col>
            </v-row>
            <v-row>
                <v-col class="px-2" cols="12">
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
                    <v-alert
                        outlined
                        type="warning"
                        text
                        transition="scale-transition"
                    >
                    Puede que pierda el acceso al equipo al modificar alguno de los siguientes parámetros. Debe reinicar el equipo para que los cambios surtan efecto.
                    </v-alert>
                </v-col>
            </v-row>
            <v-row>
                <v-col cols="12" md="6">
                    <v-form>
                        <v-row>
                            <v-col cols="12">
                                <v-text-field
                                    v-model="ssid"
                                    :counter="32"
                                    label="SSID"
                                    hint = "Red wifi a la que se intentará la conexión"
                                    persistent-hint
                                    required
                                    v-show="wifiEnabled"
                                ></v-text-field>
                            </v-col>
                            <v-col cols="12">
                                <v-text-field
                                    v-model="pwd"
                                    :append-icon="pwd == initPwd ? '' : (showPwd ? 'mdi-eye' : 'mdi-eye-off')"
                                    :type="showPwd ? 'text' : 'password'"
                                    class="input-group--focused"
                                    label="Password"
                                    persistent-hint
                                    required
                                    :counter="64"
                                    @click:append="showPwd = !showPwd"
                                    v-show="wifiEnabled"
                                ></v-text-field>
                            </v-col>
                        </v-row>
                    </v-form>
                </v-col>
                <v-col cols="12" md="6" class="d-flex">
                    <v-divider
                        class="mx-4 d-none d-md-flex"
                        vertical
                    ></v-divider>
                    <v-form>
                        <v-row align="center">
                            <v-col cols="8">
                                <v-input
                                    :messages="['Permite acceder al equipo utilizando el nombre .local']"
                                    dense
                                >
                                    Habilitar servicio mDNS
                                </v-input>
                            </v-col>
                            <v-col cols="4" class="text-right" >
                                <v-spacer></v-spacer>
                                <v-switch v-model="mDNSEnabled" flat inset dense></v-switch>
                            </v-col>
                        </v-row>
                        <v-row>
                            <v-col cols="12">
                                <v-text-field
                                    v-model="mDNSHostname"
                                    :counter="32"
                                    label="Nombre de host"
                                    hint = "Nombre para el servicio mDNS sin el sufijo .local"
                                    persistent-hint
                                    required
                                    v-show="mDNSEnabled"
                                ></v-text-field>
                            </v-col>
                        </v-row>
                    </v-form>
                </v-col>
            </v-row>
            <v-row>
                <v-col cols="12">
                    <v-spacer></v-spacer>
                    <v-btn large color="primary" v-on:click="saveInfo()">Guardar</v-btn>
                </v-col>
            </v-row>
        </v-col>
    </v-row>
</v-container>
</template>

<script>
export default {
  name: 'Config_Red',
  data: () => ({
      showPwd: false,
      success: false,
      error: false,
      mDNSEnabled: false,
      mDNSHostname: '',
      initPwd: '(vacio)',
      ssid: '',
      pwd: '(vacio)',
      wifiEnabled: false,
      current_ssid: '',
      current_canal: '',
      current_potencia: '',
      current_dns: '',
      current_gateway: '',
      current_ip: '',
      mode: ''
    }),
    methods: {
        getInfo()
        {
            var self = this;

            self.error = false;
            self.success = false;

            this.$http.get(process.env.VUE_APP_REMOTESERVER + 'red').then(function(response){
                self.mDNSEnabled = response.body["mdns_enabled"];
                self.mDNSHostname = response.body["mdns_hostname"];
                self.mode = response.body["mode"];
                self.wifiEnabled = response.body["sta_enabled"];
                self.ssid = response.body["ssid"];
                self.current_ssid = response.body["ssid"];
                self.current_canal = response.body["current_channel"];
                self.current_potencia = response.body["rssi"];
                self.current_ip = response.body["current_ip"];
                self.current_gateway = response.body["current_gateway"];
                self.current_dns = response.body["current_dns"];
            }, function(){

                self.error = true;
            });
        },
        saveInfo()
        {
            var self = this;

            self.error = false;
            self.success = false;

            var obj = 
            {
                mdns_enabled: self.mDNSEnabled,
                mdns_hostname: self.mDNSHostname,
                sta_enabled: self.wifiEnabled
            };

            if (self.ssid != self.current_ssid || self.pwd != self.initPwd)
            {
                obj["ssid"] = self.ssid;
                obj["pwd"] = self.pwd;
            }

            this.$http.post(process.env.VUE_APP_REMOTESERVER + 'red', JSON.stringify(obj), { headers: {"Content-Type": "text/plain"}}
            ).then(function(/* response */){ 
                console.log("Cambios gaurdados correctamente!");
                self.getInfo();
                self.success = true;
            }, function(){
                self.getInfo();
                self.error = true;
            });
        }
    },
    created: function(){
        this.getInfo();
    }
}
</script>

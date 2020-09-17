<template>
<v-container style="max-width: 100%">
  <v-row dark style="background-color: rgb(51,51,51); margin-top: -12px; padding-bottom: 60px">
    <v-col cols="12"
    >
      <h3 dark style="color: white">SISTEMA</h3>
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
      <p class="my-4 px-4 text-uppercase font-weight-light overline">Configuración > Sistema</p>
      <v-divider></v-divider>
      <v-alert
          outlined
          type="success"
          text
          transition="scale-transition"
          :value="success"
          v-model="success"
      >
      Equipo actualizado correctamente!. En unos segundos la página se actualizará
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
      Error al procesar la actualización.
      </v-alert>
      <v-row>
        <v-col>
          <v-card>
            <v-card-text>
              <v-row >
                <v-col cols="6" class="py-0 my-0">
                  <div class="overline text--primary">Log</div>
                </v-col>
                <v-col cols="6" class="py-0 my-0">
                  <v-checkbox @change="getLog()" v-model="showDebug" class="caption py-0 my-0" label="Mostrar mensajes Debug">
                    <template v-slot:label>
                      <span class="caption py-0 my-0">
                        Mostrar DEBUG
                      </span>
                    </template>                   
                  </v-checkbox>
                </v-col>
              </v-row>
              <v-row >
                <v-col class="py-0 my-0">
                  <div class="terminal d-flex flex-column w-100 p-2" style="overflow-y: scroll; max-height: 300px">
                    <span v-for="(entry, i) in entries" :hidden="(!showDebug && entry.level == 1) ? true : false" :key="i" class="text-caption" :class="entry.level == 1 ? 'green--text' : entry.level==4 ? 'orange--text' : entry.level == 8 ? 'red--text' : 'text--primary'">
                      {{entry.levelStr}} | {{entry.tag}} | {{entry.message}}
                    </span>
                  </div>
                </v-col>
              </v-row>
            </v-card-text>
          </v-card>
        </v-col>
      </v-row>
      <v-row>
        <v-col>
          <v-divider></v-divider>
          <h4 class="my-2 px-4">Actualiza la versión del equipo.</h4>
          <div class="d-flex flex-column justify-center">
            <div class="d-block">
              <v-file-input :class="inprogress ? 'd-none' : ''" small-chips show-size counter label="Haga click para seleccionar los ficheros de la nueva version" v-model="files"></v-file-input>
            </div>
            <div class="text-center" v-if="inprogress">
              <v-progress-linear
                color="light-blue"
                height="25"
                v-model="progress_value"
                striped
              >
                <template v-slot="{ value }">
                  <strong>{{ Math.ceil(value) }}%</strong>
                </template>
              </v-progress-linear>
            </div>
            <v-btn v-if="!inprogress" :disabled="this.files == null" :dark="true" large v-on:click="update()" color="#2196F3">Actualizar</v-btn>
          </div>
        </v-col>
      </v-row>  
    </v-col>
  </v-row>
</v-container>
</template>
<script>

import $ from 'jquery';

export default {
  name: 'Config_Reset',
  data: () => ({
      success: false,
      error: false,
      inprogress: false,
      files: null,
      progress_value: 0,
      entries: [],
      waitingLog: false,
      logTimer: null,
      showDebug: false
    }),
  methods: {
    getLog()
    {
      if (!this.waitingLog)
      {
        var self = this;
        self.waitingLog = true;

        this.$http.post(process.env.VUE_APP_REMOTESERVER + 'log', JSON.stringify({debug: self.showDebug}), { headers: {"Content-Type": "text/plain"}})
        .then(function(response){
          self.entries = response.body["entries"];
          if (self.entries !== 'undefined')
          {
            for (var i = 0; i < self.entries.length; i++)
            {
              switch (self.entries[i].level)
              {
                case 1:
                  self.entries[i].levelStr = "DEBUG";
                  break;
                case 2:
                  self.entries[i].levelStr = "INFO";
                  break;
                case 4:
                  self.entries[i].levelStr = "WARN";
                  break;
                case 8:
                  self.entries[i].levelStr = "ERROR";
                  break;
              }
              
            }

          }

          self.waitingLog = false;

        }, function(){
            console.warn("Error al recibir la actualización del log.");
            self.waitingLog = false;
        });
      }
    },
    update()
    {

      var self = this;

      var data = new FormData();
      data.append("update", this.files, this.files.name);

      self.success = false;
      self.error = false;

      $.ajax(
      {
        url: process.env.VUE_APP_REMOTESERVER + 'update',
        type: 'POST',              
        data: data,
        contentType: false,                  
        processData:false,  
        xhr: function() 
        {
          self.inprogress = true;

          var xhr = new window.XMLHttpRequest();
          xhr.upload.addEventListener('progress', function(evt) 
          {
            console.log(evt);
            if (evt.lengthComputable) 
            {
              self.inprogress = true;
              var per = evt.loaded / evt.total * 100;
              self.progress_value = per;
            }
          }, false);

          return xhr;
        },                              
        success:function(/*d, s*/) 
        {  
          self.inprogress = false;
          self.success = true;

          console.log('success!')

          window.setTimeout(function(){
              // Move to a new location or you can do something else
              window.location = "/";
          }, 5000);
        },
        error: function (/*a, b, c*/) 
        { 
          self.inprogress = false;
          self.error = true;
          self.progress_value = 0;
        }
      });
    },
    onClose(event) {
        console.log('unload');
        if (this.inprogress){
            event.preventDefault() ;   
            event.returnValue = "Espere a que se complete la actualización antes de salir";
            alert("Espere a que se complete la actualización antes de salir");
        }
    }
  },
  created: function(){
    window.addEventListener('beforeunload', this.onClose);
    this.getLog();
    this.logTimer = setInterval(this.getLog, 5000);
  },
  beforeDestroy() {
    clearInterval(this.logTimer);
  },
  beforeRouteUpdate (to, from, next) {
    // const answer = window.confirm('Do you really want to leave? you have unsaved changes!')
    // if (answer) {
    //   next()
    // } else {
    //   next(false)
    // }
    console.log("aqui");
    next(!this.inprogress);
  }
}
</script>

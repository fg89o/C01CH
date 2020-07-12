<template>
<v-container style="max-width: 100%">
  <v-row dark style="background-color: rgb(51,51,51); margin-top: -12px; padding-bottom: 60px">
    <v-col cols="12"
    >
      <h3 dark style="color: white">ACTUALIZAR</h3>
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
      <p class="my-4 px-4 text-uppercase font-weight-light overline">Configuración > Actualizar</p>
      <v-divider></v-divider>
      <h4 class="my-2 px-4">Actualiza la versión del equipo.</h4>
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
      Error al procesar el reinicio. No se recibió respuesta.
      </v-alert>
      <div class="d-flex flex-column justify-center">
        <div class="d-block">
          <v-file-input small-chips show-size counter label="Seleccione los ficheros de la nueva version" v-model="files"></v-file-input>
        </div>
        <div class="text-center" v-if="inprogress">
          <v-progress-linear
            color="light-blue"
            height="10"
            :value="progress_value"
            striped
          ></v-progress-linear>
        </div>
        <v-btn v-if="!inprogress" :disabled="this.files == null" :dark="true" large v-on:click="update()" color="#2196F3">Actualizar</v-btn>
      </div>
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
      progress_value: 0
    }),
    methods: {
      update()
      {

        var self = this;

        var data = new FormData();
        data.append("update", this.files, this.files.name);

        self.success = false;
        self.error = false;

        $.ajax(
        {
          url: '/update',
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
              if (evt.lengthComputable) 
              {
                self.inprogress = true;
                var per = evt.loaded / evt.total;
                self.progress_value = per;
              }
            }, false);
            return xhr;
          },                              
          success:function(/*d, s*/) 
          {  
            self.inprogress = true;
            self.success = true;

            self.progress_value = 0;
            console.log('success!')
          },
          error: function (/*a, b, c*/) 
          { 
            self.inprogress = false;
            self.error = true;
            self.progress_value = 0;

            window.setTimeout(function(){
                // Move to a new location or you can do something else
                window.location = "/";
            }, 5000);
          }
        });
      }
    }
}
</script>

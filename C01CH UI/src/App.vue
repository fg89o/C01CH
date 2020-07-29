<template>
  <div id="app">
    <v-app>
        <v-navigation-drawer v-model="drawer" app mobile-break-point="1024">
          <v-list class="text-center d-flex flex-column justify-center">
            <v-list-item-content>
              <v-list-item-title class="title">
                <h1><span class="blue--text">Dom</span>Dom</h1>
              </v-list-item-title>
              <v-list-item-avatar size="64" class="d-block">
                <v-img src=""></v-img>
              </v-list-item-avatar>
              <v-list-item-subtitle>
                C01CH
              </v-list-item-subtitle>
            </v-list-item-content>
          </v-list>
          <v-divider></v-divider>
          <v-list nav dense >
            <v-list-item-group v-model="menu_items_active" color="primary">
            <div
            v-for="(header) in menu_items"
            :key="header.text"
            >
              <v-subheader v-text="header.text"></v-subheader>
              
                <v-list-item link 
                v-for="(item) in header.items"
                :key="item.link"
                :to="item.link"
                >
                  <v-list-item-icon>
                        <v-icon v-text="item.icon"></v-icon>
                    </v-list-item-icon>
                  <v-list-item-content>
                    <v-list-item-title v-text="item.title"></v-list-item-title>
                  </v-list-item-content>
                </v-list-item>
              
              <v-divider></v-divider>
            </div>
            </v-list-item-group>
          </v-list>
        </v-navigation-drawer>
        <v-app-bar app color="rgb(51,51,51)" dense dark flat >
            <v-app-bar-nav-icon @click.stop="drawer = !drawer"></v-app-bar-nav-icon>
            
            <v-spacer></v-spacer>
    
            <v-btn icon target="_blank" href="https://github.com/fg89o/C01CH">
              <v-icon>mdi-github</v-icon>
            </v-btn>

            <v-btn icon target="_blank" href="https://acuariofiliamadrid.org/Thread-DIY-controlador-1-Canal?pid=530025#pid530025">
              <v-icon>mdi-help-circle</v-icon>
            </v-btn>

        </v-app-bar>
      
        <!-- Sizes your content based upon application components -->
        <v-content style="background-image: linear-gradient(to bottom, #f5f5f5 0%, white 100%);">
          <router-view></router-view>
        </v-content>
      
        <v-footer app>
            <div>{{ datenow }}</div>
            <v-spacer></v-spacer>
            <div>Óscar Fernández &copy; 2020</div>
        </v-footer>
      </v-app>
  </div>
</template>

<script>
export default {
  name: 'App',

  data: () => ({
    drawer: true,
    date: null, 
    datenow: '',
    appTitle: 'DomDom',
    menu_items: [
        {
          text: 'GENERAL',
          items: [
            { title: 'Inicio', icon: 'mdi-view-dashboard', link: '/' },
            { title: 'Programación', icon: 'mdi-clock-outline', link: '/programacion' },
            { title: 'Manual', icon: 'mdi-tune', link: '/manual' },
            { title: 'Ventilador', icon: 'mdi-fan', link: '/fan' }
          ]
        },
        {
          text: 'CONFIGURACION',
          items: [
            { title: 'Canales', icon: 'mdi-camera-iris', link: '/canales'},
            { title: 'Fecha y hora', icon: 'mdi-clock', link: '/fechaHora' },
            { title: 'Red / Wifi', icon: 'mdi-lan', link: '/red' },
            { title: 'Actualizar', icon: 'mdi-cloud-upload-outline', link: '/update'},
            { title: 'Reset', icon: 'mdi-restart', link: '/reset' },
          ]
        },
    ],
    menu_items_active: '',
    right: null,
    timeTimeout: null,
  }),

  methods: {
    time() {
        if (this.date != null)
        {
            var self = this
            this.date.setSeconds(this.date.getSeconds() + 1);

            var day = this.date.getUTCDate().toString().padStart(2,"0");
            var month = (this.date.getUTCMonth()+1).toString().padStart(2,"0");
            var year = this.date.getUTCFullYear().toString();
            var hour = this.date.getUTCHours().toString().padStart(2,"0");
            var minute = this.date.getUTCMinutes().toString().padStart(2,"0");

            this.datenow = day + "/" + month + "/" + year + " " + hour + ":" + minute;
        }


        this.timeTimeout = setTimeout(self.time, 1000)
    },
    requestTime(){
      var self = this;
      this.$http.get(this.$remoteServer + 'rtc').then(function(response){
          self.date = new Date(parseInt(response.body["unixtime"]) * 1000);

          if (self.timeTimeout != null)
          {
            window.clearTimeout(self.timeTimeout);
          }

          self.time();
      }, function(){
          console.error("Error al recibir rtc!");
      });
    },
  },
  created: function(){
    this.requestTime();
    setInterval(this.requestTime, 60000);
  },
};
</script>

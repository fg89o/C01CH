import Vue from 'vue'
import App from './App.vue'
import router from './router'
import vuetify from './plugins/vuetify';
import VueResource from "vue-resource";

Vue.use(VueResource);

Vue.config.productionTip = false

//Vue.prototype.$remoteServer = '';
Vue.prototype.$remoteServer = 'http://192.168.1.253/';

new Vue({
  router,
  vuetify,
  render: h => h(App),
  data: {
  }
}).$mount('#app')

import Vue from 'vue'
import App from './App.vue'
import router from './router'
import vuetify from './plugins/vuetify';
import VueResource from "vue-resource";

Vue.use(VueResource);

Vue.config.productionTip = false

Vue.prototype.$version = "1.1.1"


new Vue({
  router,
  vuetify,
  render: h => h(App),
  data: {
  }
}).$mount('#app')

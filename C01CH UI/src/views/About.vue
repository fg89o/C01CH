<template>
  <div class="about">
    <h1>This is an about page</h1>
  </div>
</template>
<script>
export default {
  name: 'Adj_Manual',
  data: () => ({
      canales: false,
      modo_manual: false
    }),
    methods: {
      send()
      {
        var self = this;
        var obj = {
          reset: true
        }
        this.$http.post('reset', obj).then(function(/* response */){
          self.success = true;
        }, function(){
            self.error = true;
            self.loading = false;
        });
      }
    }
}
</script>

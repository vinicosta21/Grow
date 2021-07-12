from django.db import models
from django.contrib.postgres.fields import ArrayField

class Especie(models.Model):
    nome = models.CharField(max_length=200)
    iluminacao = models.IntegerField()
    umidade = models.IntegerField()
    def __str__(self):
        return self.nome

class Planta(models.Model):
    serial = models.CharField(max_length=200, unique=True)
    umi_manual = models.IntegerField(null=True, blank=True)
    ilu_manual = models.IntegerField(null=True, blank=True)
    especie = models.ForeignKey(Especie, related_name="epecie_plantada", on_delete=models.PROTECT, blank=True, null=True)
    ilu_dados = ArrayField(models.IntegerField(), blank=True)
    umi_dados = ArrayField(models.IntegerField(), blank=True)
    ts_dados = ArrayField(models.DateTimeField(), blank=True)
    
    def __str__(self):
        return self.serial
    
    def dados(self):
        if self.especie:
            return (self.especie.iluminacao, self.especie.umidade)
        else:
            return (self.ilu_manual, self.umi_manual)
        
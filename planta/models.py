from django.db import models
from django.contrib.postgres.fields import ArrayField

class Especie(models.Model):
    nome = models.CharField(max_length=200)
    iluminacao = models.IntegerField(choices=[(0,"Baixa"), (1, "Média"), (2, "Alta")])
    umidade = models.IntegerField(choices=[(0,"Baixa"), (1, "Média"), (2, "Alta")])
    def __str__(self):
        return self.nome

class Planta(models.Model):
    serial = models.CharField(max_length=200, unique=True)
    umi_manual = models.IntegerField(choices=[(0,"Baixa"), (1, "Média"), (2, "Alta")], null=True, blank=True)
    ilu_manual = models.IntegerField(choices=[(0,"Baixa"), (1, "Média"), (2, "Alta")], null=True, blank=True)
    especie = models.ForeignKey(Especie, related_name="epecie_plantada", on_delete=models.PROTECT, blank=True, null=True)
    ilu_dados = ArrayField(models.IntegerField(), blank=True)
    umi_dados = ArrayField(models.IntegerField(), blank=True)
    ts_dados = ArrayField(models.DateTimeField(), blank=True)
    
    def __str__(self):
        return self.serial
    
    def dados(self):
        if self.especie:
            ilu = self.especie.iluminacao
            umi = self.especie.umidade
        else:
            ilu = self.ilu_manual
            umi = self.umi_manual
        ilu_dic = {0:750, 1:2000, 2:2000}
        hrs_dic = {0:8, 1:12, 2:16}
        umi_dic = {0:3300, 1:2500, 2:1500}
        return [hrs_dic[ilu], ilu_dic[ilu], umi_dic[umi]]
        
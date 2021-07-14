# Generated by Django 3.2.5 on 2021-07-14 02:32

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('planta', '0003_alter_planta_especie'),
    ]

    operations = [
        migrations.AlterField(
            model_name='especie',
            name='iluminacao',
            field=models.IntegerField(choices=[(0, 'Baixa'), (1, 'Média'), (2, 'Alta')]),
        ),
        migrations.AlterField(
            model_name='especie',
            name='umidade',
            field=models.IntegerField(choices=[(0, 'Baixa'), (1, 'Média'), (2, 'Alta')]),
        ),
        migrations.AlterField(
            model_name='planta',
            name='ilu_manual',
            field=models.IntegerField(blank=True, choices=[(0, 'Baixa'), (1, 'Média'), (2, 'Alta')], null=True),
        ),
        migrations.AlterField(
            model_name='planta',
            name='umi_manual',
            field=models.IntegerField(blank=True, choices=[(0, 'Baixa'), (1, 'Média'), (2, 'Alta')], null=True),
        ),
    ]

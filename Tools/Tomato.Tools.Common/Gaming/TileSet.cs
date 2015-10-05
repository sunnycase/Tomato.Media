//
// Tomato Media Tools
// Tile 图块
// 作者：SunnyCase
// 创建时间：2015-10-04
//
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel.DataAnnotations;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Catel.Data;
using Newtonsoft.Json;

namespace Tomato.Tools.Common.Gaming
{
    public class Property
    {
        [JsonProperty("name")]
        public string Name { get; set; }

        [JsonProperty("value")]
        public string Value { get; set; }

        public override string ToString()
        {
            return $"{Name}: {Value}";
        }
    }

    public struct Offset
    {
        [JsonProperty("x")]
        public int X { get; set; }

        [JsonProperty("y")]
        public int Y { get; set; }

        public override string ToString()
        {
            return $"{X}, {Y}";
        }
    }

    public struct Size
    {
        [JsonProperty("xlength")]
        [Range(1, int.MaxValue)]
        public int XLength { get; set; }

        [JsonProperty("zlength")]
        [Range(1, int.MaxValue)]
        public int ZLength { get; set; }

        [JsonIgnore]
        public int Count =>  XLength * ZLength;
    }

    public class Terrain
    {
        [JsonProperty("name")]
        public string Name { get; set; }

        [JsonProperty("tile")]
        public int Tile { get; set; }

        public override string ToString()
        {
            return $"{Name}: {Tile}";
        }
    }

    [JsonConverter(typeof(TerrainCornerJsonConverter))]
    public class TerrainCorner
    {
        public int TopLeft { get; set; }
        public int TopRight { get; set; }
        public int BottomLeft { get; set; }
        public int BottomRight { get; set; }

        public override string ToString()
        {
            return JsonConvert.SerializeObject(this).ToString();
        }
    }

    public class Tile
    {
        [JsonProperty("terrain")]
        public TerrainCorner Terrain { get; set; }
    }

    class TerrainCornerJsonConverter : JsonConverter
    {
        public override bool CanConvert(Type objectType)
        {
            return objectType == typeof(TerrainCorner);
        }

        public override object ReadJson(JsonReader reader, Type objectType, object existingValue, JsonSerializer serializer)
        {
            var corner = new TerrainCorner();
            if (reader.TokenType != JsonToken.StartArray) throw new JsonReaderException();
            corner.TopLeft = reader.ReadAsInt32() ?? 0;
            corner.TopRight = reader.ReadAsInt32() ?? 0;
            corner.BottomLeft = reader.ReadAsInt32() ?? 0;
            corner.BottomRight = reader.ReadAsInt32() ?? 0;
            reader.Read();
            if (reader.TokenType != JsonToken.EndArray) throw new JsonReaderException();
            return corner;
        }

        public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer)
        {
            var corner = (TerrainCorner)value;
            writer.WriteStartArray();
            writer.WriteValue(corner.TopLeft);
            writer.WriteValue(corner.TopRight);
            writer.WriteValue(corner.BottomLeft);
            writer.WriteValue(corner.BottomRight);
            writer.WriteEndArray();
        }
    }

    public class TileUnitElementPosition
    {
        [JsonProperty("x")]
        public int X { get; set; }

        [JsonProperty("z")]
        public int Z { get; set; }

        public override string ToString()
        {
            return $"{X}, {Z}";
        }
    }

    public class TileUnitElement
    {
        [JsonProperty("position")]
        public TileUnitElementPosition Position { get; set; }

        [JsonProperty("tile")]
        public int Tile { get; set; }

        [JsonProperty("height")]
        public int Height { get; set; }
    }

    public class TileUnit
    {
        [JsonProperty("size")]
        public Size Size { get; set; }

        [JsonProperty("tiles")]
        public ObservableCollection<TileUnitElement> Tiles { get; set; }

        [JsonProperty("category")]
        [Required]
        public string Category { get; set; }

        public TileUnit()
        {
            Tiles = new ObservableCollection<TileUnitElement>();
        }
    }

    public class PickAnyTileUnitElement
    {
        [JsonProperty("tile")]
        public int Tile { get; set; }

        [JsonProperty("height")]
        public int Height { get; set; }
    }

    public class PickAnyTileUnit
    {
        [JsonProperty("size")]
        public Size Size { get; set; }

        [JsonProperty("tiles")]
        public ObservableCollection<PickAnyTileUnitElement> Tiles { get; set; }

        [JsonProperty("category")]
        [Required]
        public string Category { get; set; }

        public PickAnyTileUnit()
        {
            Tiles = new ObservableCollection<PickAnyTileUnitElement>();
        }
    }

    /// <summary>
    /// Tile 图块
    /// </summary>
    public class TileSet
    {
        [JsonProperty("name")]
        public string Name { get; set; }

        [JsonProperty("image")]
        public string Image { get; set; }

        [JsonProperty("imagewidth")]
        public int ImageWidth { get; set; }

        [JsonProperty("imageheight")]
        public int ImageHeight { get; set; }

        [JsonProperty("tilewidth")]
        public int TileWidth { get; set; }

        [JsonProperty("tileheight")]
        public int TileHeight { get; set; }

        [JsonProperty("tilecount")]
        public int TileCount { get; set; }
        
        [JsonProperty("tileoffset")]
        public Offset TileOffset { get; set; }

        [JsonProperty("terrains")]
        public ObservableCollection<Terrain> Terrains { get; set; }

        [JsonProperty("tiles")]
        public Dictionary<int, Tile> Tiles { get; set; }

        [JsonProperty("tileunits")]
        public ObservableCollection<TileUnit> TileUnits { get; set; }

        [JsonProperty("pickanytileunits")]
        public ObservableCollection<PickAnyTileUnit> PickAnyTileUnits { get; set; }

        public TileSet()
        {
            Terrains = new ObservableCollection<Terrain>();
            Tiles = new Dictionary<int, Tile>();
            TileUnits = new ObservableCollection<TileUnit>();
            PickAnyTileUnits = new ObservableCollection<PickAnyTileUnit>();
        }
    }
}

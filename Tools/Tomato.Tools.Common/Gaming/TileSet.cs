//
// Tomato Media Tools
// Tile 图块
// 作者：SunnyCase
// 创建时间：2015-10-04
//
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
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
    public struct TerrainCorner
    {
        public bool TopLeft { get; set; }
        public bool TopRight { get; set; }
        public bool BottomLeft { get; set; }
        public bool BottomRight { get; set; }

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
            corner.TopLeft = reader.ReadAsInt32() != 0;
            corner.TopRight = reader.ReadAsInt32() != 0;
            corner.BottomLeft = reader.ReadAsInt32() != 0;
            corner.BottomRight = reader.ReadAsInt32() != 0;
            reader.Read();
            if (reader.TokenType != JsonToken.EndArray) throw new JsonReaderException();
            return corner;
        }

        public override void WriteJson(JsonWriter writer, object value, JsonSerializer serializer)
        {
            var corner = (TerrainCorner)value;
            writer.WriteStartArray();
            writer.WriteValue(BooleanToInt32(corner.TopLeft));
            writer.WriteValue(BooleanToInt32(corner.TopRight));
            writer.WriteValue(BooleanToInt32(corner.BottomLeft));
            writer.WriteValue(BooleanToInt32(corner.BottomRight));
            writer.WriteEndArray();
        }

        private static int BooleanToInt32(bool value)
        {
            return value ? 1 : 0;
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
        public List<Terrain> Terrains { get; set; } = new List<Terrain>();

        [JsonProperty("tiles")]
        public Dictionary<int, Tile> Tiles { get; set; } = new Dictionary<int, Tile>();
    }
}
